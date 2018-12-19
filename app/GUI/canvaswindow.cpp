#include "canvaswindow.h"
#include "canvas.h"
#include <QComboBox>
#include "mainwindow.h"
#include "GUI/BoxesList/boxscrollwidgetvisiblepart.h"
#include "singlewidgetabstraction.h"
#include "taskexecutor.h"
#include "renderoutputwidget.h"
#include "Sound/soundcomposition.h"
#include "global.h"
#include "renderinstancesettings.h"
#include "newcanvasdialog.h"
#include "Boxes/videobox.h"
#include "Boxes/imagebox.h"
#include "Sound/singlesound.h"
#include "svgimporter.h"
#include "filesourcescache.h"
#include <QFileDialog>
#include "windowsinglewidgettarget.h"
#include "videoencoder.h"
#include "usagewidget.h"
#include "memorychecker.h"

CanvasWindow::CanvasWindow(QWidget *parent) {
    mWindowSWTTarget = SPtrCreate(WindowSingleWidgetTarget)(this);
    //setAttribute(Qt::WA_OpaquePaintEvent, true);

    mPreviewFPSTimer = new QTimer(this);

    initializeAudio();

    mCanvasWidget = QWidget::createWindowContainer(this, parent);
    mCanvasWidget->setAcceptDrops(true);

    mCanvasWidget->setMouseTracking(true);


    VideoEncoderEmitter *vidEmitter = VideoEncoder::getVideoEncoderEmitter();
//    connect(vidEmitter, &VideoEncoderEmitter::encodingStarted,
//            this, &CanvasWindow::leaveOnlyInterruptionButtonsEnabled);
    connect(vidEmitter, &VideoEncoderEmitter::encodingFinished,
            this, &CanvasWindow::interruptOutputRendering);
    connect(vidEmitter, &VideoEncoderEmitter::encodingInterrupted,
            this, &CanvasWindow::interruptOutputRendering);
    connect(vidEmitter, &VideoEncoderEmitter::encodingFailed,
            this, &CanvasWindow::interruptOutputRendering);
    connect(vidEmitter, &VideoEncoderEmitter::encodingStartFailed,
            this, &CanvasWindow::interruptOutputRendering);
}

CanvasWindow::~CanvasWindow() {}

Canvas *CanvasWindow::getCurrentCanvas() {
    return mCurrentCanvas;
}

void CanvasWindow::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction *abstraction,
        const UpdateFuncs &updateFuncs,
        const int& visiblePartWidgetId) {
    Q_FOREACH(const qsptr<Canvas> &child, mCanvasList) {
        auto abs = child->SWT_getAbstractionForWidget(updateFuncs,
                                                      visiblePartWidgetId);
        abstraction->addChildAbstraction(abs);
    }
}

void CanvasWindow::setCurrentCanvas(const int &id) {
    if(id < 0 || id >= mCanvasList.count()) {
        setCurrentCanvas(nullptr);
    } else {
        setCurrentCanvas(mCanvasList.at(id).get());
    }
}

void CanvasWindow::setCurrentCanvas(Canvas * const canvas) {
    TaskScheduler::sSetCurrentCanvas(canvas);
    if(mCurrentCanvas != nullptr) {
        mCurrentCanvas->setIsCurrentCanvas(false);
        disconnect(mPreviewFPSTimer, &QTimer::timeout,
                   mCurrentCanvas.data(), &Canvas::nextPreviewFrame);
    }

    if(canvas == nullptr) {
        mCurrentSoundComposition = nullptr;
        mCurrentCanvas.clear();
    } else {
        mCurrentCanvas = canvas;
        mCurrentSoundComposition = mCurrentCanvas->getSoundComposition();
        connect(mPreviewFPSTimer, &QTimer::timeout,
                mCurrentCanvas.data(), &Canvas::nextPreviewFrame);

        mCurrentCanvas->setIsCurrentCanvas(true);

        setCanvasMode(mCurrentCanvas->getCurrentCanvasMode());

        emit changeCanvasFrameRange(0, getMaxFrame());
        emit changeCurrentFrame(getCurrentFrame());
    }
    BoxesGroup *currentGroup;
    if(mCurrentCanvas == nullptr) {
        MainWindow::getInstance()->setCurrentUndoRedoStack(nullptr);
        currentGroup = nullptr;
    } else {
        MainWindow::getInstance()->setCurrentUndoRedoStack(
                    mCurrentCanvas->getUndoRedoStack());
        currentGroup = mCurrentCanvas->getCurrentBoxesGroup();
    }
    mWindowSWTTarget->SWT_scheduleWidgetsContentUpdateWithTarget(
                currentGroup,
                SWT_CurrentGroup);
    mWindowSWTTarget->SWT_scheduleWidgetsContentUpdateWithTarget(
                mCurrentCanvas.data(),
                SWT_CurrentCanvas);
    MainWindow::getInstance()->updateSettingsForCurrentCanvas();
    callUpdateSchedulers();
}

void CanvasWindow::addCanvasToList(const qsptr<Canvas>& canvas) {
    mCanvasList << canvas;
    mWindowSWTTarget->SWT_addChildAbstractionForTargetToAll(canvas.get());
}

void CanvasWindow::removeCanvas(const int &id) {
    qsptr<Canvas> canvas = mCanvasList.takeAt(id);
    mWindowSWTTarget->SWT_removeChildAbstractionForTargetFromAll(canvas.data());
    if(mCanvasList.isEmpty()) {
        setCurrentCanvas(nullptr);
    } else if(id < mCanvasList.count()) {
        setCurrentCanvas(id);
    } else {
        setCurrentCanvas(id - 1);
    }
}

void CanvasWindow::setCanvasMode(const CanvasMode &mode) {
    if(hasNoCanvas()) {
        setCursor(QCursor(Qt::ArrowCursor) );
        return;
    }

    if(mode == MOVE_PATH) {
        setCursor(QCursor(Qt::ArrowCursor) );
    } else if(mode == MOVE_POINT) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-node.xpm"), 0, 0) );
    } else if(mode == PICK_PAINT_SETTINGS) {
        setCursor(QCursor(QPixmap(":/cursors/cursor_color_picker.png"), 2, 20) );
    } else if(mode == ADD_CIRCLE) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-ellipse.xpm"), 4, 4) );
    } else if(mode == ADD_RECTANGLE ||
              mode == ADD_PARTICLE_BOX ||
              mode == ADD_PAINT_BOX) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-rect.xpm"), 4, 4) );
    } else if(mode == ADD_TEXT) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-text.xpm"), 4, 4) );
    } else if(mode == PAINT_MODE) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-crosshairs.xpm"), 4, 4) );
    } else {
        setCursor(QCursor(QPixmap(":/cursors/cursor-pen.xpm"), 4, 4) );
    }

    mCurrentCanvas->setCanvasMode(mode);
    MainWindow::getInstance()->updateCanvasModeButtonsChecked();
}

void CanvasWindow::callUpdateSchedulers() {
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
}

void CanvasWindow::setMovePathMode() {
    setCanvasMode(MOVE_PATH);
}

void CanvasWindow::setMovePointMode() {
    setCanvasMode(MOVE_POINT);
}

void CanvasWindow::setAddPointMode() {
    setCanvasMode(ADD_POINT);
}

void CanvasWindow::setRectangleMode() {
    setCanvasMode(ADD_RECTANGLE);
}

void CanvasWindow::setPickPaintSettingsMode() {
    setCanvasMode(PICK_PAINT_SETTINGS);
}

void CanvasWindow::setCircleMode() {
    setCanvasMode(ADD_CIRCLE);
}

void CanvasWindow::setTextMode() {
    setCanvasMode(ADD_TEXT);
}

void CanvasWindow::setParticleBoxMode() {
    setCanvasMode(ADD_PARTICLE_BOX);
}

void CanvasWindow::setParticleEmitterMode() {
    setCanvasMode(ADD_PARTICLE_EMITTER);
}

void CanvasWindow::setPaintBoxMode() {
    setCanvasMode(ADD_PAINT_BOX);
}

void CanvasWindow::setPaintMode() {
    setCanvasMode(PAINT_MODE);
}

void CanvasWindow::addCanvasToListAndSetAsCurrent(
        const qsptr<Canvas>& canvas) {
    addCanvasToList(canvas);
    setCurrentCanvas(canvas.get());
}

void CanvasWindow::renameCanvas(Canvas *canvas,
                                const QString &newName) {
    canvas->setName(newName);
}

void CanvasWindow::renameCanvas(const int &id,
                                const QString &newName) {
    renameCanvas(mCanvasList.at(id).data(), newName);
}

bool CanvasWindow::hasNoCanvas() {
    return mCurrentCanvas == nullptr;
}

void CanvasWindow::renameCurrentCanvas(const QString &newName) {
    if(mCurrentCanvas == nullptr) return;
    renameCanvas(mCurrentCanvas.data(), newName);
}

void CanvasWindow::qRender(QPainter *p) {
    Q_UNUSED(p);
    if(mCurrentCanvas == nullptr) return;
    //mCurrentCanvas->drawInputText(p);
}
#include "glhelpers.h"

void CanvasWindow::renderSk(SkCanvas * const canvas,
                            GrContext* const grContext) {
    if(mCurrentCanvas == nullptr) {
        canvas->clear(SK_ColorBLACK);
        return;
    }
    mCurrentCanvas->renderSk(canvas, grContext);
}

void CanvasWindow::tabletEvent(QTabletEvent *e) {
    if(hasNoCanvas()) return;
    QPoint global_pos = mapToGlobal( QPoint(0, 0) );
    qreal w_x_t = e->hiResGlobalX() - global_pos.x();
    qreal w_y_t = e->hiResGlobalY() - global_pos.y();
    mCurrentCanvas->tabletEvent(e, QPointF(w_x_t, w_y_t));
}

void CanvasWindow::mousePressEvent(QMouseEvent *event) {
    KFT_setFocus();
    if(mCurrentCanvas == nullptr) return;
    mCurrentCanvas->mousePressEvent(event);
}

void CanvasWindow::mouseReleaseEvent(QMouseEvent *event) {
    if(mCurrentCanvas == nullptr) return;
    mCurrentCanvas->mouseReleaseEvent(event);
}

void CanvasWindow::mouseMoveEvent(QMouseEvent *event) {
    if(mCurrentCanvas == nullptr) return;
    mCurrentCanvas->mouseMoveEvent(event);
}

void CanvasWindow::wheelEvent(QWheelEvent *event) {
    if(mCurrentCanvas == nullptr) return;
    mCurrentCanvas->wheelEvent(event);
}

void CanvasWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    if(mCurrentCanvas == nullptr) return;
    mCurrentCanvas->mouseDoubleClickEvent(event);
}

void CanvasWindow::openSettingsWindowForCurrentCanvas() {
    if(hasNoCanvas()) return;
    CanvasSettingsDialog *dialog =
            new CanvasSettingsDialog(mCurrentCanvas.data(),
                                     MainWindow::getInstance());
    connect(dialog, &QDialog::accepted, this, [dialog, this]() {
        dialog->applySettingsToCanvas(mCurrentCanvas.data());
        this->setCurrentCanvas(mCurrentCanvas.data());
    });
    dialog->show();

//    if(dialog->exec() == QDialog::Accepted) {
//        dialog->applySettingsToCanvas(mCurrentCanvas.data());
//        setCurrentCanvas(mCurrentCanvas.data());
//    }
}

void CanvasWindow::rotate90CCW() {
    if(hasNoCanvas()) return;
    //mCurrentCanvas->rotate90CCW();
}

void CanvasWindow::rotate90CW() {
    if(hasNoCanvas()) return;
    //mCurrentCanvas->rotate90CW();
}

bool CanvasWindow::KFT_handleKeyEventForTarget(QKeyEvent *event) {
    if(hasNoCanvas()) return false;
    if(event->key() == Qt::Key_F1) {
        setCanvasMode(CanvasMode::MOVE_PATH);
    } else if(event->key() == Qt::Key_F2) {
        setCanvasMode(CanvasMode::MOVE_POINT);
    } else if(event->key() == Qt::Key_F3) {
        setCanvasMode(CanvasMode::ADD_POINT);
    } else if(event->key() == Qt::Key_F4) {
        setCanvasMode(CanvasMode::PICK_PAINT_SETTINGS);
    } else if(event->key() == Qt::Key_F6) {
        setCanvasMode(CanvasMode::ADD_CIRCLE);
    } else if(event->key() == Qt::Key_F7) {
        setCanvasMode(CanvasMode::ADD_RECTANGLE);
    } else if(event->key() == Qt::Key_F8) {
        setCanvasMode(CanvasMode::ADD_TEXT);
    } else if(event->key() == Qt::Key_F9) {
        setCanvasMode(CanvasMode::ADD_PARTICLE_BOX);
    } else if(event->key() == Qt::Key_F10) {
        setCanvasMode(CanvasMode::ADD_PARTICLE_EMITTER);
    } else if(event->key() == Qt::Key_F11) {
        setCanvasMode(CanvasMode::ADD_PAINT_BOX);
    } else if(event->key() == Qt::Key_F12) {
        setCanvasMode(CanvasMode::PAINT_MODE);
    } else if(event->key() == Qt::Key_F13) {
        //setCanvasMode(CanvasMode::ADD_BONE);
    } else if(mCurrentCanvas->keyPressEvent(event)) {
    } else {
        return false;
    }
    callUpdateSchedulers();
    return true;
}

void CanvasWindow::raiseAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->raiseSelectedBoxes();
    callUpdateSchedulers();
}

void CanvasWindow::lowerAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->lowerSelectedBoxes();
    callUpdateSchedulers();
}

void CanvasWindow::raiseToTopAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->raiseSelectedBoxesToTop();
    callUpdateSchedulers();
}

void CanvasWindow::lowerToBottomAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->lowerSelectedBoxesToBottom();
    callUpdateSchedulers();
}

void CanvasWindow::objectsToPathAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->convertSelectedBoxesToPath();
    callUpdateSchedulers();
}

void CanvasWindow::strokeToPathAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->convertSelectedPathStrokesToPath();
    callUpdateSchedulers();
}

void CanvasWindow::rotate90CWAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->rotateSelectedBoxesStartAndFinish(90.);
    callUpdateSchedulers();
}

void CanvasWindow::rotate90CCWAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->rotateSelectedBoxesStartAndFinish(-90.);
    callUpdateSchedulers();
}

void CanvasWindow::flipHorizontalAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->flipSelectedBoxesHorizontally();
    callUpdateSchedulers();
}

void CanvasWindow::flipVerticalAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->flipSelectedBoxesVertically();
    callUpdateSchedulers();
}

void CanvasWindow::setCurrentBrush(const Brush *brush) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setCurrentBrush(brush);
}

void CanvasWindow::replaceBrush(const Brush *oldBrush,
                                const Brush *newBrush) {
    foreach(const qsptr<Canvas> &canvas, mCanvasList) {
        if(canvas->getCurrentBrush() == oldBrush) {
            canvas->setCurrentBrush(newBrush);
        }
    }
}

void CanvasWindow::pathsUnionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsUnion();
    callUpdateSchedulers();
}

void CanvasWindow::pathsDifferenceAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsDifference();
    callUpdateSchedulers();
}

void CanvasWindow::pathsIntersectionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsIntersection();
    callUpdateSchedulers();
}

void CanvasWindow::pathsDivisionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsDivision();
    callUpdateSchedulers();
}

void CanvasWindow::pathsExclusionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsExclusion();
    callUpdateSchedulers();
}

void CanvasWindow::pathsCombineAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsCombine();
    callUpdateSchedulers();
}

void CanvasWindow::pathsBreakApartAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsBreakApart();
    callUpdateSchedulers();
}

void CanvasWindow::setFontFamilyAndStyle(const QString& family,
                                         const QString& style) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFontFamilyAndStyle(family, style);
    callUpdateSchedulers();
}

void CanvasWindow::setFontSize(const qreal& size) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFontSize(size);
    callUpdateSchedulers();
}

void CanvasWindow::connectPointsSlot() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->connectPoints();
    callUpdateSchedulers();
}

void CanvasWindow::disconnectPointsSlot() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->disconnectPoints();
    callUpdateSchedulers();
}

void CanvasWindow::mergePointsSlot() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->mergePoints();
    callUpdateSchedulers();
}

void CanvasWindow::makePointCtrlsSymmetric() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makePointCtrlsSymmetric();
    callUpdateSchedulers();
}

void CanvasWindow::makePointCtrlsSmooth() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makePointCtrlsSmooth();
    callUpdateSchedulers();
}

void CanvasWindow::makePointCtrlsCorner() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makePointCtrlsCorner();
    callUpdateSchedulers();
}

void CanvasWindow::makeSegmentLine() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makeSegmentLine();
    callUpdateSchedulers();
}

void CanvasWindow::makeSegmentCurve() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makeSegmentCurve();
    callUpdateSchedulers();
}

void CanvasWindow::startSelectedStrokeWidthTransform() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->startSelectedStrokeWidthTransform();
    callUpdateSchedulers();
}

void CanvasWindow::deleteAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->deleteAction();
}

void CanvasWindow::copyAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->copyAction();
}

void CanvasWindow::pasteAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->pasteAction();
}

void CanvasWindow::cutAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->cutAction();
}

void CanvasWindow::duplicateAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->duplicateAction();
}

void CanvasWindow::selectAllAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectAllAction();
}

void CanvasWindow::invertSelectionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->invertSelectionAction();
}

void CanvasWindow::clearSelectionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->clearSelectionAction();
}

void CanvasWindow::groupSelectedBoxes() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->groupSelectedBoxes();
    callUpdateSchedulers();
}

void CanvasWindow::ungroupSelectedBoxes() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->ungroupSelectedBoxes();
    callUpdateSchedulers();
}

void CanvasWindow::startSelectedStrokeColorTransform() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->startSelectedStrokeColorTransform();
    callUpdateSchedulers();
}

void CanvasWindow::startSelectedFillColorTransform() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->startSelectedFillColorTransform();
    callUpdateSchedulers();
}

void CanvasWindow::strokeCapStyleChanged(const Qt::PenCapStyle &capStyle) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedCapStyle(capStyle);
    callUpdateSchedulers();
}

void CanvasWindow::strokeJoinStyleChanged(const Qt::PenJoinStyle &joinStyle) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedJoinStyle(joinStyle);
    callUpdateSchedulers();
}

void CanvasWindow::strokeWidthChanged(const qreal &strokeWidth,
                                      const bool &finish) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeWidth(strokeWidth, finish);
    callUpdateSchedulers();
}

void CanvasWindow::applyPaintSettingToSelected(PaintSetting *setting) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->applyPaintSettingToSelected(setting);
}

void CanvasWindow::setSelectedFillColorMode(const ColorMode &mode) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFillColorMode(mode);
}

void CanvasWindow::setSelectedStrokeColorMode(const ColorMode &mode) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeColorMode(mode);
}

void CanvasWindow::updateAfterFrameChanged(const int &currentFrame) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->prp_setAbsFrame(currentFrame);
}

void CanvasWindow::getDisplayedFillStrokeSettingsFromLastSelected(
        PaintSettings*& fillSetings, StrokeSettings*& strokeSettings) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->getDisplayedFillStrokeSettingsFromLastSelected(
                fillSetings, strokeSettings);
}

void CanvasWindow::setClipToCanvas(const bool &bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setClipToCanvas(bT);
    mCurrentCanvas->updateAllBoxes(Animator::USER_CHANGE);
    callUpdateSchedulers();
}

void CanvasWindow::setRasterEffectsVisible(const bool &bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setRasterEffectsVisible(bT);
    mCurrentCanvas->updateAllBoxes(Animator::USER_CHANGE);
    callUpdateSchedulers();
}

void CanvasWindow::setPathEffectsVisible(const bool &bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setPathEffectsVisible(bT);
    mCurrentCanvas->updateAllBoxes(Animator::USER_CHANGE);
    callUpdateSchedulers();
}

void CanvasWindow::setResolutionFraction(const qreal &percent) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setResolutionFraction(percent);
    mCurrentCanvas->clearAllCache();
    mCurrentCanvas->updateAllBoxes(Animator::USER_CHANGE);
    callUpdateSchedulers();
}

void CanvasWindow::updatePivotIfNeeded() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->updatePivotIfNeeded();
}

void CanvasWindow::schedulePivotUpdate() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->schedulePivotUpdate();
}

BoxesGroup *CanvasWindow::getCurrentGroup() {
    if(hasNoCanvas()) return nullptr;
    return mCurrentCanvas->getCurrentBoxesGroup();
}

void CanvasWindow::renderFromSettings(RenderInstanceSettings *settings) {
    VideoEncoder::startEncodingStatic(settings);
    if(VideoEncoder::encodingSuccessfulyStartedStatic()) {
        mSavedCurrentFrame = getCurrentFrame();
        mSavedResolutionFraction = mCurrentCanvas->getResolutionFraction();

        mCurrentRenderSettings = settings;
        const RenderSettings &renderSettings = settings->getRenderSettings();
        Canvas *canvas = settings->getTargetCanvas();
        setCurrentCanvas(canvas);
        emit changeCurrentFrame(renderSettings.minFrame);

        qreal resolutionFraction = renderSettings.resolution;
        mMaxRenderFrame = renderSettings.maxFrame;

        auto cpuFinishedFunc = [this]() {
            nextSaveOutputFrame();
        };
        TaskScheduler::sSetAllQuedCPUTasksFinishedFunc(cpuFinishedFunc);

        mCurrentCanvas->fitCanvasToSize();
        if(qAbs(mSavedResolutionFraction - resolutionFraction) > 0.00001) {
            mCurrentCanvas->setResolutionFraction(resolutionFraction);
        }

        mCurrentRenderFrame = renderSettings.minFrame;
        mCurrentCanvas->prp_setAbsFrame(mCurrentRenderFrame);
        mCurrentCanvas->setOutputRendering(true);
        mCurrentCanvas->updateAllBoxes(Animator::USER_CHANGE);
        if(TaskScheduler::sAllQuedCPUTasksFinished()) {
            nextSaveOutputFrame();
        }
    }
}

void CanvasWindow::nextCurrentRenderFrame() {
    int newCurrentRenderFrame = mCurrentCanvas->getCacheHandler().
            getFirstEmptyOrCachedFrameAfterFrame(mCurrentRenderFrame);
    int firstIdT;
    int lastIdT;
    mCurrentCanvas->prp_getFirstAndLastIdenticalRelFrame(&firstIdT,
                                                         &lastIdT,
                                                         newCurrentRenderFrame);
    if(mCurrentRenderFrame >= firstIdT) {
        newCurrentRenderFrame = lastIdT + 1;
    } else {
        newCurrentRenderFrame = firstIdT;
    }
    if(newCurrentRenderFrame - mCurrentRenderFrame > 1) {
        mCurrentCanvas->getCacheHandler().
            setContainersInFrameRangeBlocked(mCurrentRenderFrame + 1,
                                             newCurrentRenderFrame - 1,
                                             true);
    }

    mCurrentRenderFrame = newCurrentRenderFrame;
    emit changeCurrentFrame(mCurrentRenderFrame);
}

void CanvasWindow::renderPreview() {
    if(hasNoCanvas()) return;
    auto cpuFinishedFunc = [this]() {
        nextPreviewRenderFrame();
    };
    TaskScheduler::sSetAllQuedCPUTasksFinishedFunc(cpuFinishedFunc);

    mSavedCurrentFrame = getCurrentFrame();

    mCurrentRenderFrame = mSavedCurrentFrame;
    mMaxRenderFrame =
            mCurrentCanvas->getMaxPreviewFrame(mCurrentRenderFrame,
                                               getMaxFrame());
    mMaxRenderFrame = getMaxFrame();
    setRendering(true);

    //mCurrentCanvas->prp_setAbsFrame(mSavedCurrentFrame);
    //mCurrentCanvas->updateAllBoxes();
    //callUpdateSchedulers();
    if(TaskScheduler::sAllQuedCPUTasksFinished()) {
        nextPreviewRenderFrame();
    }
    MainWindow::getInstance()->previewBeingRendered();
}

void CanvasWindow::interruptPreview() {
    if(mRenderingPreview) {
        interruptPreviewRendering();
    } else if(mPreviewing) {
        stopPreview();
    }
}

void CanvasWindow::outOfMemory() {
    if(mRenderingPreview) {
        if(TaskScheduler::sAllQuedCPUTasksFinished()) {
            playPreview();
        } else {
            auto allFinishedFunc = [this]() {
                playPreview();
                TaskScheduler::sSetAllQuedCPUTasksFinishedFunc(nullptr);
            };
            TaskScheduler::sSetAllQuedCPUTasksFinishedFunc(allFinishedFunc);
        }
    }
}

void CanvasWindow::setRendering(const bool &bT) {
    mRenderingPreview = bT;
    mCurrentCanvas->setRenderingPreview(bT);
}

void CanvasWindow::setPreviewing(const bool &bT) {
    mPreviewing = bT;
    mCurrentCanvas->setPreviewing(bT);
}

void CanvasWindow::interruptPreviewRendering() {
    setRendering(false);
    TaskScheduler::sClearAllFinishedFuncs();
    mCurrentCanvas->clearPreview();
    mCurrentCanvas->getCacheHandler().
        setContainersInFrameRangeBlocked(mSavedCurrentFrame + 1,
                                         mMaxRenderFrame,
                                         false);
    emit changeCurrentFrame(mSavedCurrentFrame);
    MainWindow::getInstance()->previewFinished();
}

void CanvasWindow::interruptOutputRendering() {
    mCurrentCanvas->setOutputRendering(false);
    TaskScheduler::sClearAllFinishedFuncs();
    mCurrentCanvas->clearPreview();
    emit changeCurrentFrame(mSavedCurrentFrame);
}

void CanvasWindow::stopPreview() {
    setPreviewing(false);
    mCurrentCanvas->getCacheHandler().
        setContainersInFrameRangeBlocked(mSavedCurrentFrame + 1,
                                         mMaxRenderFrame,
                                         false);
    emit changeCurrentFrame(mSavedCurrentFrame);
    mPreviewFPSTimer->stop();
    stopAudio();
    requestUpdate();
    MainWindow::getInstance()->previewFinished();
}

void CanvasWindow::pausePreview() {
    if(mPreviewing) {
        mPreviewFPSTimer->stop();
        MainWindow::getInstance()->previewPaused();
    }
}

void CanvasWindow::resumePreview() {
    if(mPreviewing) {
        mPreviewFPSTimer->start();
        MainWindow::getInstance()->previewBeingPlayed();
    }
}

void CanvasWindow::playPreview() {
    //emit changeCurrentFrame(mSavedCurrentFrame);
    TaskScheduler::sClearAllFinishedFuncs();
    mCurrentCanvas->playPreview(mSavedCurrentFrame,
                                mCurrentRenderFrame);
    setRendering(false);
    setPreviewing(true);
    mCurrentSoundComposition->generateData(mSavedCurrentFrame,
                                           mCurrentRenderFrame,
                                           mCurrentCanvas->getFps());
    startAudio();
    int mSecInterval = qRound(1000/mCurrentCanvas->getFps());
    mPreviewFPSTimer->setInterval(mSecInterval);
    mPreviewFPSTimer->start();
    MainWindow::getInstance()->previewBeingPlayed();
    requestUpdate();
}

void CanvasWindow::nextPreviewRenderFrame() {
    //mCurrentCanvas->renderCurrentFrameToPreview();
    if(!mRenderingPreview) return;
    if(mCurrentRenderFrame >= mMaxRenderFrame) {
        playPreview();
    } else {
        nextCurrentRenderFrame();
        if(TaskScheduler::sAllQuedCPUTasksFinished()) {
            nextPreviewRenderFrame();
        }
    }
}

void CanvasWindow::nextSaveOutputFrame() {
    //mCurrentCanvas->renderCurrentFrameToOutput(*mCurrentRenderSettings);
    if(mCurrentRenderFrame >= mMaxRenderFrame) {
        mCurrentRenderSettings = nullptr;
        TaskScheduler::sClearAllFinishedFuncs();
        mCurrentCanvas->setOutputRendering(false);
        emit changeCurrentFrame(mSavedCurrentFrame);
        if(qAbs(mSavedResolutionFraction -
                mCurrentCanvas->getResolutionFraction()) > 0.1) {
            mCurrentCanvas->setResolutionFraction(mSavedResolutionFraction);
        }
        VideoEncoder::finishEncodingStatic();
    } else {
        int lastIdentical;
        int firstIdentical;
        mCurrentRenderSettings->setCurrentRenderFrame(mCurrentRenderFrame);
        mCurrentCanvas->prp_getFirstAndLastIdenticalRelFrame(
                    &firstIdentical, &lastIdentical, mCurrentRenderFrame);
        if(lastIdentical > mMaxRenderFrame) lastIdentical = mMaxRenderFrame;
        mCurrentRenderFrame = lastIdentical + 1;
        //mCurrentRenderFrame++;
        emit changeCurrentFrame(mCurrentRenderFrame);
        if(TaskScheduler::sAllQuedCPUTasksFinished()) {
            // mCurrentCanvas->setCurrentPreviewContainer(); !!!
            nextSaveOutputFrame();
        }
    }
}

void CanvasWindow::clearAll() {
    foreach(const qsptr<Canvas> &canvas, mCanvasList) {
        mWindowSWTTarget->SWT_removeChildAbstractionForTargetFromAll(canvas.data());
    }

    mCanvasList.clear();
    setCurrentCanvas(nullptr);
}

void CanvasWindow::createLinkToFileWithPath(const QString &path) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->createLinkToFileWithPath(path);
}

void CanvasWindow::createAnimationBoxForPaths(
        const QStringList &importPaths) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->createAnimationBoxForPaths(importPaths);
}

VideoBox *CanvasWindow::createVideoForPath(const QString &path) {
    if(hasNoCanvas()) return nullptr;
    return mCurrentCanvas->createVideoForPath(path);
}

ImageBox *CanvasWindow::createImageForPath(const QString &path) {
    if(hasNoCanvas()) return nullptr;
    return mCurrentCanvas->createImageBox(path);
}

SingleSound *CanvasWindow::createSoundForPath(const QString &path) {
    if(hasNoCanvas()) return nullptr;
    return mCurrentCanvas->createSoundForPath(path);
}

int CanvasWindow::getCurrentFrame() {
    if(hasNoCanvas()) return 0;
    return mCurrentCanvas->getCurrentFrame();
}

int CanvasWindow::getMaxFrame() {
    if(hasNoCanvas()) return 0;
    return mCurrentCanvas->getMaxFrame();
}

void CanvasWindow::updateHoveredElements() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->updateHoveredElements();
}

void CanvasWindow::setLocalPivot(const bool &bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setLocalPivot(bT);
}

void CanvasWindow::setBonesSelectionEnabled(const bool &bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setBonesSelectionEnabled(bT);
}

const int BufferSize = 32768;

void CanvasWindow::initializeAudio() {
    mAudioBuffer = QByteArray(BufferSize, 0);
    connect(mPreviewFPSTimer, &QTimer::timeout,
            this, &CanvasWindow::pushTimerExpired);

    mAudioDevice = QAudioDeviceInfo::defaultOutputDevice();
    mAudioFormat.setSampleRate(SOUND_SAMPLERATE);
    mAudioFormat.setChannelCount(1);
    mAudioFormat.setSampleSize(32);
    mAudioFormat.setCodec("audio/pcm");
    mAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
    mAudioFormat.setSampleType(QAudioFormat::Float);

    QAudioDeviceInfo info(mAudioDevice);
    if(!info.isFormatSupported(mAudioFormat)) {
        qWarning() << "Default format not supported - trying to use nearest";
        mAudioFormat = info.nearestFormat(mAudioFormat);
    }

    mAudioOutput = new QAudioOutput(mAudioDevice, mAudioFormat, this);
}

void CanvasWindow::startAudio() {
    mCurrentSoundComposition->start();
    mAudioIOOutput = mAudioOutput->start();
}

void CanvasWindow::stopAudio() {
    //mAudioOutput->suspend();
    //mCurrentSoundComposition->stop();
    mAudioIOOutput = nullptr;
    mAudioOutput->stop();
    mAudioOutput->reset();
    mCurrentSoundComposition->stop();
}

void CanvasWindow::volumeChanged(int value) {
    if(mAudioOutput) {
        mAudioOutput->setVolume(qreal(value/100.));
    }
}

void CanvasWindow::pushTimerExpired() {
    if(mAudioOutput && mAudioOutput->state() != QAudio::StoppedState) {
        int chunks = mAudioOutput->bytesFree()/mAudioOutput->periodSize();
        while(chunks) {
           const qint64 len = mCurrentSoundComposition->read(
                                                mAudioBuffer.data(),
                                                mAudioOutput->periodSize());
           if(len) {
               mAudioIOOutput->write(mAudioBuffer.data(), len);
           }
           if(len != mAudioOutput->periodSize()) {
               break;
           }
           --chunks;
        }
    }
}

void CanvasWindow::dropEvent(QDropEvent *event) {
    const QMimeData* mimeData = event->mimeData();

    if(mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();

        for(int i = 0; i < urlList.size() && i < 32; i++) {
            importFile(urlList.at(i).toLocalFile(),
                       mCurrentCanvas->mapCanvasAbsToRel(event->posF()));
        }
    }
}

void CanvasWindow::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CanvasWindow::importFile(const QString &path,
                              const QPointF &relDropPos) {
    if(hasNoCanvas()) return;

    QFile file(path);
    if(!file.exists()) {
        return;
    }
    MainWindow::getInstance()->disable();

    QString extension = path.split(".").last();
    if(isSoundExt(extension)) {
        createSoundForPath(path);
    } else {
        qsptr<BoundingBox> importedBox;
        mCurrentCanvas->blockUndoRedo();
        if(isVectorExt(extension)) {
            importedBox = loadSVGFile(path);
        } else if(isImageExt(extension)) {
            qsptr<ImageBox> imgBox = SPtrCreate(ImageBox)();
            importedBox = GetAsSPtr(imgBox, BoundingBox);
            imgBox->setFilePath(path);
        } else if(isVideoExt(extension)) {
            qsptr<VideoBox> vidBox = SPtrCreate(VideoBox)();
            importedBox = GetAsSPtr(vidBox, BoundingBox);
            vidBox->setFilePath(path);
        } else if(isAvExt(extension)) {
            MainWindow::getInstance()->loadAVFile(path);
        }
        mCurrentCanvas->unblockUndoRedo();

        if(importedBox != nullptr) {
            mCurrentCanvas->getCurrentBoxesGroup()->addContainedBox(
                        importedBox);
            QPointF trans = relDropPos;
            trans -= importedBox->mapRelPosToAbs(
                        importedBox->getRelCenterPosition());
            importedBox->moveByAbs(trans);
        }
        updateHoveredElements();
    }
    MainWindow::getInstance()->enable();
}

QWidget *CanvasWindow::getCanvasWidget() {
    return mCanvasWidget;
}

void CanvasWindow::grabMouse() {
    mMouseGrabber = true;
#ifndef QT_DEBUG
    setMouseGrabEnabled(true);
#endif
}

void CanvasWindow::releaseMouse() {
    mMouseGrabber = false;
#ifndef QT_DEBUG
    setMouseGrabEnabled(false);
#endif
}

bool CanvasWindow::isMouseGrabber() {
    return mMouseGrabber;
}

QRect CanvasWindow::rect() {
    return mCanvasWidget->rect();
}

void CanvasWindow::importFile() {
    MainWindow::getInstance()->disableEventFilter();
    QStringList importPaths = QFileDialog::getOpenFileNames(
                                            MainWindow::getInstance(),
                                            "Import File", "",
                                            "Files (*.av *.svg "
                                                   "*.mp4 *.mov *.avi *.mkv *.m4v "
                                                   "*.png *.jpg "
                                                   "*.wav *.mp3)");
    MainWindow::getInstance()->enableEventFilter();
    if(!importPaths.isEmpty()) {
        Q_FOREACH(const QString &path, importPaths) {
            if(path.isEmpty()) continue;
            importFile(path);
        }
    }
}

void CanvasWindow::startDurationRectPosTransformForAllSelected() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->startDurationRectPosTransformForAllSelected();
}

void CanvasWindow::finishDurationRectPosTransformForAllSelected() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->finishDurationRectPosTransformForAllSelected();
}

void CanvasWindow::moveDurationRectForAllSelected(const int &dFrame) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->moveDurationRectForAllSelected(dFrame);
}

void CanvasWindow::startMinFramePosTransformForAllSelected() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->startMinFramePosTransformForAllSelected();
}

void CanvasWindow::finishMinFramePosTransformForAllSelected() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->finishMinFramePosTransformForAllSelected();
}

void CanvasWindow::moveMinFrameForAllSelected(const int &dFrame) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->moveMinFrameForAllSelected(dFrame);
}

void CanvasWindow::startMaxFramePosTransformForAllSelected() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->startMaxFramePosTransformForAllSelected();
}

void CanvasWindow::finishMaxFramePosTransformForAllSelected() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->finishMaxFramePosTransformForAllSelected();
}

void CanvasWindow::moveMaxFrameForAllSelected(const int &dFrame) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->moveMaxFrameForAllSelected(dFrame);
}
