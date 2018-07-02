#include "canvaswindow.h"
#include "canvas.h"
#include <QComboBox>
#include "mainwindow.h"
#include "BoxesList/boxscrollwidgetvisiblepart.h"
#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"
#include "paintcontroler.h"
#include "renderoutputwidget.h"
#include "Sound/soundcomposition.h"
#include "global.h"
#include "canvaswidget.h"
#include "RenderWidget/renderinstancesettings.h"
#include "newcanvasdialog.h"
#include "Boxes/videobox.h"
#include "Boxes/imagebox.h"
#include "Sound/singlesound.h"
#include "svgimporter.h"
#include "filesourcescache.h"
#include <QFileDialog>
#include "windowsinglewidgettarget.h"
#include "videoencoder.h"

CanvasWindow::CanvasWindow(QWidget *parent) {
    mWindowSWTTarget = new WindowSingleWidgetTarget(this);
    //setAttribute(Qt::WA_OpaquePaintEvent, true);
    int numberThreads = qMax(1, QThread::idealThreadCount());
    for(int i = 0; i < numberThreads; i++) {
        QThread *paintControlerThread = new QThread(this);
        PaintControler *paintControler = new PaintControler(i);
        paintControler->moveToThread(paintControlerThread);
        connect(paintControler, SIGNAL(finishedUpdating(int, _Executor*)),
                this, SLOT(sendNextUpdatableForUpdate(int, _Executor*)) );
        connect(this, SIGNAL(updateUpdatable(_Executor*, int)),
                paintControler, SLOT(updateUpdatable(_Executor*, int)) );

        paintControlerThread->start();

        mPaintControlers << paintControler;
        mControlerThreads << paintControlerThread;

        mFreeThreads << i;
    }

    mFileControlerThread = new QThread(this);
    mFileControler = new PaintControler(numberThreads);
    mFileControler->moveToThread(mFileControlerThread);
    connect(mFileControler, SIGNAL(finishedUpdating(int, _Executor*)),
            this, SLOT(sendNextFileUpdatableForUpdate(int, _Executor*)) );
    connect(this, SIGNAL(updateFileUpdatable(_Executor*, int)),
            mFileControler, SLOT(updateUpdatable(_Executor*, int)) );

    mFileControlerThread->start();
    mControlerThreads << mFileControlerThread;

    mPreviewFPSTimer = new QTimer(this);

    initializeAudio();

    mCanvasWidget = QWidget::createWindowContainer(this, parent);
    //mCanvasWidget = new CanvasWidget(this, parent);
    mCanvasWidget->setAcceptDrops(true);
    mCanvasWidget->setMinimumSize(MIN_WIDGET_HEIGHT*10,
                                  MIN_WIDGET_HEIGHT*10);
    mCanvasWidget->setSizePolicy(QSizePolicy::Minimum,
                                 QSizePolicy::Minimum);
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

CanvasWindow::~CanvasWindow() {
    foreach(QThread *thread, mControlerThreads) {
        thread->quit();
        thread->wait();
        delete thread;
    }
    foreach(PaintControler *paintControler, mPaintControlers) {
        delete paintControler;
    }
//    mFileControlerThread->quit();
//    mFileControlerThread->wait();
    delete mFileControler;
    delete mWindowSWTTarget;
}

Canvas *CanvasWindow::getCurrentCanvas() {
    return mCurrentCanvas.data();
}

void CanvasWindow::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction *abstraction,
        ScrollWidgetVisiblePart *visiblePartWidget) {
    Q_FOREACH(const CanvasQSPtr &child, mCanvasList) {
        abstraction->addChildAbstraction(
                    child->SWT_getAbstractionForWidget(visiblePartWidget));
    }
}

void CanvasWindow::setCurrentCanvas(const int &id) {
    if(id < 0 || id >= mCanvasList.count()) {
        setCurrentCanvas((Canvas*)nullptr);
    } else {
        setCurrentCanvas(mCanvasList.at(id).data());
    }
}

void CanvasWindow::setCurrentCanvas(Canvas *canvas) {
    if(mCurrentCanvas != nullptr) {
        mCurrentCanvas->setIsCurrentCanvas(false);
        disconnect(mPreviewFPSTimer, SIGNAL(timeout()),
                   mCurrentCanvas.data(), SLOT(nextPreviewFrame()) );
    }

    if(canvas == nullptr) {
        mCurrentSoundComposition = nullptr;
        mCurrentCanvas.reset();
    } else {
        mCurrentCanvas = canvas->ref<Canvas>();
        mCurrentSoundComposition = mCurrentCanvas->getSoundComposition();
        connect(mPreviewFPSTimer, SIGNAL(timeout()),
                mCurrentCanvas.data(), SLOT(nextPreviewFrame()) );

        mCurrentCanvas->setIsCurrentCanvas(true);

        setCanvasMode(mCurrentCanvas->getCurrentCanvasMode());

        emit changeFrameRange(0, getMaxFrame());
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
    updateDisplayedFillStrokeSettings();
    MainWindow::getInstance()->updateSettingsForCurrentCanvas();
    callUpdateSchedulers();
}

void CanvasWindow::addCanvasToList(Canvas *canvas) {
    mCanvasList << canvas->ref<Canvas>();
    mWindowSWTTarget->SWT_addChildAbstractionForTargetToAll(canvas);
}

void CanvasWindow::removeCanvas(const int &id) {
    CanvasQSPtr canvas = mCanvasList.takeAt(id);
    mWindowSWTTarget->SWT_removeChildAbstractionForTargetFromAll(canvas.data());
    if(mCanvasList.isEmpty()) {
        setCurrentCanvas((Canvas*)nullptr);
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
    MainWindow::getInstance()->callUpdateSchedulers();
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

void CanvasWindow::addCanvasToListAndSetAsCurrent(Canvas *canvas) {
    addCanvasToList(canvas);
    setCurrentCanvas(canvas);
}

void CanvasWindow::renameCanvas(Canvas *canvas, const QString &newName) {
    canvas->setName(newName);
}

void CanvasWindow::renameCanvas(const int &id, const QString &newName) {
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
    if(mCurrentCanvas == nullptr) return;
    //mCurrentCanvas->drawInputText(p);
}

void CanvasWindow::renderSk(SkCanvas *canvas) {
    if(mCurrentCanvas == nullptr) {
        canvas->clear(SK_ColorBLACK);
        return;
    }
    mCurrentCanvas->renderSk(canvas);
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
    foreach(const CanvasQSPtr &canvas, mCanvasList) {
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

void CanvasWindow::setFontFamilyAndStyle(QString family, QString style) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFontFamilyAndStyle(family, style);
    callUpdateSchedulers();
}

void CanvasWindow::setFontSize(qreal size) {
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

void CanvasWindow::applyPaintSettingToSelected(
        const PaintSetting &setting) {
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

void CanvasWindow::updateDisplayedFillStrokeSettings() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setDisplayedFillStrokeSettingsFromLastSelected();
}

void CanvasWindow::setClipToCanvas(const bool &bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setClipToCanvas(bT);
    mCurrentCanvas->updateAllBoxes();
    callUpdateSchedulers();
}

void CanvasWindow::setRasterEffectsVisible(const bool &bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setRasterEffectsVisible(bT);
    mCurrentCanvas->updateAllBoxes();
    callUpdateSchedulers();
}

void CanvasWindow::setPathEffectsVisible(const bool &bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setPathEffectsVisible(bT);
    mCurrentCanvas->updateAllBoxes();
    callUpdateSchedulers();
}

void CanvasWindow::setResolutionFraction(const qreal &percent) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setResolutionFraction(percent);
    mCurrentCanvas->clearAllCache();
    mCurrentCanvas->updateAllBoxes();
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
#include "videoencoder.h"
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
        mBoxesUpdateFinishedFunction = &CanvasWindow::nextSaveOutputFrame;
        mCurrentCanvas->fitCanvasToSize();
        if(qAbs(mSavedResolutionFraction - resolutionFraction) > 0.00001) {
            mCurrentCanvas->setResolutionFraction(resolutionFraction);
        }

        mCurrentRenderFrame = renderSettings.minFrame;
        mCurrentCanvas->prp_setAbsFrame(mCurrentRenderFrame);
        mCurrentCanvas->setOutputRendering(true);
        mCurrentCanvas->updateAllBoxes();
        if(mNoBoxesAwaitUpdate) {
            nextSaveOutputFrame();
        }
    }
}

void CanvasWindow::nextCurrentRenderFrame() {
    int newCurrentRenderFrame = mCurrentCanvas->getCacheHandler()->
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
        mCurrentCanvas->getCacheHandler()->
            setContainersInFrameRangeBlocked(mCurrentRenderFrame + 1,
                                             newCurrentRenderFrame - 1,
                                             true);
    }

    mCurrentRenderFrame = newCurrentRenderFrame;
    emit changeCurrentFrame(mCurrentRenderFrame);
}

void CanvasWindow::renderPreview() {
    if(hasNoCanvas()) return;
    mBoxesUpdateFinishedFunction = &CanvasWindow::nextPreviewRenderFrame;
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
    if(mNoBoxesAwaitUpdate) {
        nextPreviewRenderFrame();
    }
    MainWindow::getInstance()->previewBeingRendered();
}

void CanvasWindow::processSchedulers() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->processSchedulers();
    mCurrentCanvas->addSchedulersToProcess();
}

bool CanvasWindow::shouldProcessAwaitingSchedulers() {
    return mNoBoxesAwaitUpdate;// || !mFreeThreads.isEmpty();
}

void CanvasWindow::addUpdatableAwaitingUpdate(_Executor *updatable) {
    if(mNoBoxesAwaitUpdate) {
        mNoBoxesAwaitUpdate = false;
    }

    mUpdatablesAwaitingUpdate << updatable->ref<_Executor>();
    if(!mFreeThreads.isEmpty()) {
        sendNextUpdatableForUpdate(mFreeThreads.takeFirst(), nullptr);
    }
}

void CanvasWindow::addFileUpdatableAwaitingUpdate(_Executor *updatable) {
    mFileUpdatablesAwaitingUpdate << updatable->ref<_Executor>();

    if(mNoFileAwaitUpdate) {
        mNoFileAwaitUpdate = false;
        sendNextFileUpdatableForUpdate(mPaintControlers.count(), nullptr);
    }
}

void CanvasWindow::sendNextFileUpdatableForUpdate(const int &threadId,
                                              _Executor *lastUpdatable) {
    Q_UNUSED(threadId);
    if(lastUpdatable != nullptr) {
        lastUpdatable->updateFinished();
    }
    if(mFileUpdatablesAwaitingUpdate.isEmpty()) {
        mNoFileAwaitUpdate = true;
        if(mFilesUpdateFinishedFunction != nullptr) {
            (*this.*mFilesUpdateFinishedFunction)();
        }
        if(!mRendering) {
            callUpdateSchedulers();
        }
        if(!mFreeThreads.isEmpty() && !mUpdatablesAwaitingUpdate.isEmpty()) {
            sendNextUpdatableForUpdate(mFreeThreads.takeFirst(), nullptr);
        }
    } else {
        for(int i = 0; i < mFileUpdatablesAwaitingUpdate.count(); i++) {
            _Executor *updatablaT = mFileUpdatablesAwaitingUpdate.at(i).get();
            if(updatablaT->readyToBeProcessed()) {
                updatablaT->setCurrentPaintControler(mFileControler);
                updatablaT->beforeUpdate();
                emit updateFileUpdatable(updatablaT, mPaintControlers.count());
                mFileUpdatablesAwaitingUpdate.removeAt(i);
                i--;
                return;
            }
        }
    }
}

void CanvasWindow::sendNextUpdatableForUpdate(const int &threadId,
                                              _Executor *lastUpdatable) {
    if(lastUpdatable != nullptr) {
        if(mClearBeingUpdated) {
            lastUpdatable->clear();
        } else {
            lastUpdatable->updateFinished();
        }
    }
    if(mUpdatablesAwaitingUpdate.isEmpty()) {
        mClearBeingUpdated = false;
        mNoBoxesAwaitUpdate = true;
        mFreeThreads << threadId;
        if(mBoxesUpdateFinishedFunction != nullptr) {
            (*this.*mBoxesUpdateFinishedFunction)();
        }
        if(!mRendering) {
            callUpdateSchedulers();
        }
    } else {
        for(int i = 0; i < mUpdatablesAwaitingUpdate.count(); i++) {
            _Executor *updatablaT = mUpdatablesAwaitingUpdate.at(i).get();
            if(updatablaT->readyToBeProcessed()) {
                updatablaT->setCurrentPaintControler(
                            mPaintControlers.at(threadId));
                updatablaT->beforeUpdate();
                emit updateUpdatable(updatablaT, threadId);
                mUpdatablesAwaitingUpdate.removeAt(i);
                i--;
                return;
            }
        }
        mFreeThreads << threadId;
    }
}

void CanvasWindow::interruptPreview() {
    if(mRendering) {
        interruptPreviewRendering();
    } else if(mPreviewing) {
        stopPreview();
    }
}

void CanvasWindow::outOfMemory() {
    if(mRendering) {
        if(mNoBoxesAwaitUpdate && mNoFileAwaitUpdate) {
            playPreview();
        } else if(mNoBoxesAwaitUpdate) {
            mBoxesUpdateFinishedFunction = nullptr;
            mFilesUpdateFinishedFunction = &CanvasWindow::playPreview;
        } else if(mNoFileAwaitUpdate) {
            mFilesUpdateFinishedFunction = nullptr;
            mBoxesUpdateFinishedFunction = &CanvasWindow::playPreview;
        }
    }
}

void CanvasWindow::setRendering(const bool &bT) {
    mRendering = bT;
    mCurrentCanvas->setRenderingPreview(bT);
}
#include "memorychecker.h"
void CanvasWindow::setPreviewing(const bool &bT) {
    mPreviewing = bT;
    mCurrentCanvas->setPreviewing(bT);
}

void CanvasWindow::interruptPreviewRendering() {
    setRendering(false);
    mBoxesUpdateFinishedFunction = nullptr;
    mFilesUpdateFinishedFunction = nullptr;
    mCurrentCanvas->clearPreview();
    mCurrentCanvas->getCacheHandler()->
        setContainersInFrameRangeBlocked(mSavedCurrentFrame + 1,
                                         mMaxRenderFrame,
                                         false);
    emit changeCurrentFrame(mSavedCurrentFrame);
    MainWindow::getInstance()->previewFinished();
}

void CanvasWindow::interruptOutputRendering() {
    mCurrentCanvas->setOutputRendering(false);
    mBoxesUpdateFinishedFunction = nullptr;
    mFilesUpdateFinishedFunction = nullptr;
    mCurrentCanvas->clearPreview();
    emit changeCurrentFrame(mSavedCurrentFrame);
}

void CanvasWindow::stopPreview() {
    setPreviewing(false);
    mCurrentCanvas->getCacheHandler()->
        setContainersInFrameRangeBlocked(mSavedCurrentFrame + 1,
                                         mMaxRenderFrame,
                                         false);
    emit changeCurrentFrame(mSavedCurrentFrame);
    mPreviewFPSTimer->stop();
    stopAudio();
    repaint();
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
    mBoxesUpdateFinishedFunction = nullptr;
    mFilesUpdateFinishedFunction = nullptr;
    mCurrentCanvas->playPreview(mSavedCurrentFrame,
                                mCurrentRenderFrame);
    setRendering(false);
    setPreviewing(true);
    mCurrentSoundComposition->generateData(mSavedCurrentFrame,
                                           mCurrentRenderFrame,
                                           mCurrentCanvas->getFps());
    startAudio();
    mPreviewFPSTimer->setInterval(1000/mCurrentCanvas->getFps());
    mPreviewFPSTimer->start();
    MainWindow::getInstance()->previewBeingPlayed();
    requestUpdate();
}

void CanvasWindow::nextPreviewRenderFrame() {
    //mCurrentCanvas->renderCurrentFrameToPreview();
    if(!mRendering) return;
    if(mCurrentRenderFrame >= mMaxRenderFrame) {
        playPreview();
    } else {
        nextCurrentRenderFrame();
        if(mNoBoxesAwaitUpdate) {
            nextPreviewRenderFrame();
        }
    }
}

void CanvasWindow::nextSaveOutputFrame() {
    mCurrentCanvas->renderCurrentFrameToOutput(*mCurrentRenderSettings);
    if(mCurrentRenderFrame >= mMaxRenderFrame) {
        mCurrentRenderSettings = nullptr;
        mBoxesUpdateFinishedFunction = nullptr;
        mFilesUpdateFinishedFunction = nullptr;
        mCurrentCanvas->setOutputRendering(false);
        emit changeCurrentFrame(mSavedCurrentFrame);
        if(qAbs(mSavedResolutionFraction -
                mCurrentCanvas->getResolutionFraction()) > 0.1) {
            mCurrentCanvas->setResolutionFraction(mSavedResolutionFraction);
        }
        VideoEncoder::finishEncodingStatic();
    } else {
        mCurrentRenderSettings->setCurrentRenderFrame(mCurrentRenderFrame);
        mCurrentRenderFrame++;
        emit changeCurrentFrame(mCurrentRenderFrame);
        if(mNoBoxesAwaitUpdate) {
            // mCurrentCanvas->setCurrentPreviewContainer(); !!!
            nextSaveOutputFrame();
        }
    }
}

void CanvasWindow::clearAll() {
    //SWT_clearAll();

    mClearBeingUpdated = !mNoBoxesAwaitUpdate;
    mUpdatablesAwaitingUpdate.clear();
    foreach(const CanvasQSPtr &canvas, mCanvasList) {
        mWindowSWTTarget->SWT_removeChildAbstractionForTargetFromAll(canvas.data());
    }

    mCanvasList.clear();
    setCurrentCanvas((Canvas*)nullptr);
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
    connect(mPreviewFPSTimer, SIGNAL(timeout()),
            this, SLOT(pushTimerExpired()));

    mAudioDevice = QAudioDeviceInfo::defaultOutputDevice();
    mAudioFormat.setSampleRate(SOUND_SAMPLERATE);
    mAudioFormat.setChannelCount(1);
    mAudioFormat.setSampleSize(32);
    mAudioFormat.setCodec("audio/pcm");
    mAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
    mAudioFormat.setSampleType(QAudioFormat::Float);

    QAudioDeviceInfo info(mAudioDevice);
    if (!info.isFormatSupported(mAudioFormat)) {
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

        for (int i = 0; i < urlList.size() && i < 32; i++) {
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
        QSharedPointer<BoundingBox> importedBox;
        mCurrentCanvas->blockUndoRedo();
        if(isVectorExt(extension)) {
            importedBox = loadSVGFile(path)->ref<BoundingBox>();
        } else if(isImageExt(extension)) {
            ImageBox *imgBox = new ImageBox();
            importedBox = imgBox->ref<BoundingBox>();
            imgBox->setFilePath(path);
        } else if(isVideoExt(extension)) {
            VideoBox *vidBox = new VideoBox();
            importedBox = vidBox->ref<BoundingBox>();
            vidBox->setFilePath(path);
        } else if(isAvExt(extension)) {
            MainWindow::getInstance()->loadAVFile(path);
        }
        mCurrentCanvas->unblockUndoRedo();

        if(importedBox != nullptr) {
            mCurrentCanvas->getCurrentBoxesGroup()->addContainedBox(
                        importedBox.data());
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

void CanvasWindow::repaint() {
    //mCanvasWidget->update();
    requestUpdate();
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
