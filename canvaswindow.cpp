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

CanvasWindow::CanvasWindow(QWidget *parent) {
    mWindowSWTTarget = new WindowSingleWidgetTarget(this);
    //setAttribute(Qt::WA_OpaquePaintEvent, true);
    int numberThreads = qMax(1, QThread::idealThreadCount());
    for(int i = 0; i < numberThreads; i++) {
        QThread *paintControlerThread = new QThread(this);
        PaintControler *paintControler = new PaintControler(i);
        paintControler->moveToThread(paintControlerThread);
        connect(paintControler, SIGNAL(finishedUpdating(int, Executable*)),
                this, SLOT(sendNextUpdatableForUpdate(int, Executable*)) );
        connect(this, SIGNAL(updateUpdatable(Executable*, int)),
                paintControler, SLOT(updateUpdatable(Executable*, int)) );

        paintControlerThread->start();

        mPaintControlers << paintControler;
        mControlerThreads << paintControlerThread;

        mFreeThreads << i;
    }

    mFileControlerThread = new QThread(this);
    mFileControler = new PaintControler(numberThreads);
    mFileControler->moveToThread(mFileControlerThread);
    connect(mFileControler, SIGNAL(finishedUpdating(int, Executable*)),
            this, SLOT(sendNextFileUpdatableForUpdate(int, Executable*)) );
    connect(this, SIGNAL(updateFileUpdatable(Executable*, int)),
            mFileControler, SLOT(updateUpdatable(Executable*, int)) );

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
    mFileControlerThread->quit();
    mFileControlerThread->wait();
    delete mFileControler;
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
        setCurrentCanvas((Canvas*)NULL);
    } else {
        setCurrentCanvas(mCanvasList.at(id).data());
    }
}

void CanvasWindow::setCurrentCanvas(Canvas *canvas) {
    if(mCurrentCanvas != NULL) {
        mCurrentCanvas->setIsCurrentCanvas(false);
        disconnect(mPreviewFPSTimer, SIGNAL(timeout()),
                   mCurrentCanvas.data(), SLOT(nextPreviewFrame()) );
    }

    if(canvas == NULL) {
        mCurrentSoundComposition = NULL;
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
        setCurrentCanvas((Canvas*)NULL);
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
    } else if(mode == PICK_PATH_SETTINGS) {
        setCursor(QCursor(QPixmap(":/cursors/cursor_color_picker.png"), 2, 20) );
    } else if(mode == DRAW_PATH) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-pen.xpm"), 4, 4) );
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

void CanvasWindow::setDrawPathMode() {
    setCanvasMode(DRAW_PATH);
}

void CanvasWindow::setAddDrawPathNodeMode() {
    setCanvasMode(ADD_DRAW_PATH_NODE);
}

void CanvasWindow::setRectangleMode() {
    setCanvasMode(ADD_RECTANGLE);
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
    return mCurrentCanvas == NULL;
}

void CanvasWindow::renameCurrentCanvas(const QString &newName) {
    if(mCurrentCanvas == NULL) return;
    renameCanvas(mCurrentCanvas.data(), newName);
}

void CanvasWindow::qRender(QPainter *p) {
    if(mCurrentCanvas == NULL) return;
    //mCurrentCanvas->drawInputText(p);
}

void CanvasWindow::renderSk(SkCanvas *canvas) {
    if(mCurrentCanvas == NULL) {
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
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->mousePressEvent(event);
}

void CanvasWindow::mouseReleaseEvent(QMouseEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->mouseReleaseEvent(event);
}

void CanvasWindow::mouseMoveEvent(QMouseEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->mouseMoveEvent(event);
}

void CanvasWindow::wheelEvent(QWheelEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->wheelEvent(event);
}

void CanvasWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    if(mCurrentCanvas == NULL) return;
    mCurrentCanvas->mouseDoubleClickEvent(event);
}

void CanvasWindow::openSettingsWindowForCurrentCanvas() {
    if(hasNoCanvas()) return;
    CanvasSettingsDialog dialog(mCurrentCanvas.data(), mCanvasWidget);

    if(dialog.exec() == QDialog::Accepted) {
        dialog.applySettingsToCanvas(mCurrentCanvas.data());
        setCurrentCanvas(mCurrentCanvas.data());
    }
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
        setCanvasMode(CanvasMode::DRAW_PATH);
    } else if(event->key() == Qt::Key_F5) {
        setCanvasMode(CanvasMode::ADD_DRAW_PATH_NODE);
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

void CanvasWindow::pickPathForSettings(const bool &pickFill,
                                       const bool &pickStroke) {
    if(hasNoCanvas()) return;
    setCanvasMode(PICK_PATH_SETTINGS);
    mCurrentCanvas->setPickingFromPath(pickFill, pickStroke);
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
    if(hasNoCanvas()) return NULL;
    return mCurrentCanvas->getCurrentBoxesGroup();
}

void CanvasWindow::renderOutput() {
    QSize size = mCurrentCanvas->getCanvasSize();
    RenderOutputWidget *dialog = new RenderOutputWidget(
                                        size.width(),
                                        size.height(),
                                        MainWindow::getInstance());
    connect(dialog, SIGNAL(renderOutput(QString, qreal)),
            this, SLOT(saveOutput(QString, qreal)));
    dialog->exec();
}

void CanvasWindow::renderFromSettings(RenderInstanceSettings *settings) {
    mCurrentRenderSettings = settings;
    Canvas *canvas = settings->getTargetCanvas();
    const QString &destination = settings->getOutputDestination();
    setCurrentCanvas(canvas);
    emit changeCurrentFrame(0);
    saveOutput(destination, 1.);
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

bool CanvasWindow::noBoxesAwaitUpdate() {
    return mNoBoxesAwaitUpdate;
}

void CanvasWindow::addUpdatableAwaitingUpdate(Executable *updatable) {
    if(mNoBoxesAwaitUpdate) {
        mNoBoxesAwaitUpdate = false;
    }

    mUpdatablesAwaitingUpdate << updatable->ref<Executable>();
    if(!mFreeThreads.isEmpty()) {
        sendNextUpdatableForUpdate(mFreeThreads.takeFirst(), NULL);
    }
}

void CanvasWindow::addFileUpdatableAwaitingUpdate(Executable *updatable) {
    mFileUpdatablesAwaitingUpdate << updatable->ref<Executable>();

    if(mNoFileAwaitUpdate) {
        mNoFileAwaitUpdate = false;
        sendNextFileUpdatableForUpdate(mPaintControlers.count(), NULL);
    }
}

void CanvasWindow::sendNextFileUpdatableForUpdate(const int &threadId,
                                              Executable *lastUpdatable) {
    Q_UNUSED(threadId);
    if(lastUpdatable != NULL) {
        lastUpdatable->updateFinished();
    }
    if(mFileUpdatablesAwaitingUpdate.isEmpty()) {
        mNoFileAwaitUpdate = true;
        if(mFilesUpdateFinishedFunction != NULL) {
            (*this.*mFilesUpdateFinishedFunction)();
        }
        if(!mRendering) {
            callUpdateSchedulers();
        }
        if(!mFreeThreads.isEmpty() && !mUpdatablesAwaitingUpdate.isEmpty()) {
            sendNextUpdatableForUpdate(mFreeThreads.takeFirst(), NULL);
        }
    } else {
        for(int i = 0; i < mFileUpdatablesAwaitingUpdate.count(); i++) {
            Executable *updatablaT = mFileUpdatablesAwaitingUpdate.at(i).get();
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
                                              Executable *lastUpdatable) {
    if(lastUpdatable != NULL) {
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
        if(mBoxesUpdateFinishedFunction != NULL) {
            (*this.*mBoxesUpdateFinishedFunction)();
        }
        if(!mRendering) {
            callUpdateSchedulers();
        }
    } else {
        for(int i = 0; i < mUpdatablesAwaitingUpdate.count(); i++) {
            Executable *updatablaT = mUpdatablesAwaitingUpdate.at(i).get();
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
        interruptRendering();
    } else if(mPreviewing) {
        stopPreview();
    }
}

void CanvasWindow::outOfMemory() {
    if(mRendering) {
        if(mNoBoxesAwaitUpdate && mNoFileAwaitUpdate) {
            playPreview();
        } else if(mNoBoxesAwaitUpdate) {
            mBoxesUpdateFinishedFunction = NULL;
            mFilesUpdateFinishedFunction = &CanvasWindow::playPreview;
        } else if(mNoFileAwaitUpdate) {
            mFilesUpdateFinishedFunction = NULL;
            mBoxesUpdateFinishedFunction = &CanvasWindow::playPreview;
        }
    }
}

void CanvasWindow::setRendering(const bool &bT) {
    mRendering = bT;
    mCurrentCanvas->setRendering(bT);
}
#include "memorychecker.h"
void CanvasWindow::setPreviewing(const bool &bT) {
    mPreviewing = bT;
    mCurrentCanvas->setPreviewing(bT);
}

void CanvasWindow::interruptRendering() {
    setRendering(false);
    mBoxesUpdateFinishedFunction = NULL;
    mFilesUpdateFinishedFunction = NULL;
    mCurrentCanvas->clearPreview();
    mCurrentCanvas->getCacheHandler()->
        setContainersInFrameRangeBlocked(mSavedCurrentFrame + 1,
                                         mMaxRenderFrame,
                                         false);
    emit changeCurrentFrame(mSavedCurrentFrame);
    MainWindow::getInstance()->previewFinished();
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
    mBoxesUpdateFinishedFunction = NULL;
    mFilesUpdateFinishedFunction = NULL;
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
    mCurrentCanvas->renderCurrentFrameToOutput(mOutputString);
    if(mCurrentRenderFrame > mMaxRenderFrame) {
        emit changeCurrentFrame(mSavedCurrentFrame);
        mCurrentRenderSettings = NULL;
        mBoxesUpdateFinishedFunction = NULL;
        mFilesUpdateFinishedFunction = NULL;
        mCurrentCanvas->setOutputRendering(false);
        mCurrentCanvas->clearCurrentPreviewImage();
        if(qAbs(mSavedResolutionFraction -
                mCurrentCanvas->getResolutionFraction()) > 0.1) {
            mCurrentCanvas->setResolutionFraction(mSavedResolutionFraction);
        }
    } else {
        mCurrentRenderSettings->setCurrentRenderFrame(
                    mCurrentRenderFrame);
        mCurrentRenderFrame++;
        emit changeCurrentFrame(mCurrentRenderFrame);
        if(mNoBoxesAwaitUpdate) {
            nextSaveOutputFrame();
        }
    }
}

void CanvasWindow::saveOutput(const QString &renderDest,
                              const qreal &resolutionFraction) {
    mOutputString = renderDest;
    mMaxRenderFrame = getMaxFrame();
    mBoxesUpdateFinishedFunction = &CanvasWindow::nextSaveOutputFrame;
    mSavedCurrentFrame = getCurrentFrame();
    mCurrentCanvas->fitCanvasToSize();
    mSavedResolutionFraction = mCurrentCanvas->getResolutionFraction();
    if(qAbs(mSavedResolutionFraction - resolutionFraction) > 0.001) {
        mCurrentCanvas->setResolutionFraction(resolutionFraction);
    }

    mCurrentRenderFrame = mSavedCurrentFrame;
    mCurrentCanvas->prp_setAbsFrame(mSavedCurrentFrame);
    mCurrentCanvas->setOutputRendering(true);
    mCurrentCanvas->updateAllBoxes();
    if(mNoBoxesAwaitUpdate) {
        nextSaveOutputFrame();
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
    setCurrentCanvas((Canvas*)NULL);
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
    if(hasNoCanvas()) return NULL;
    return mCurrentCanvas->createVideoForPath(path);
}

ImageBox *CanvasWindow::createImageForPath(const QString &path) {
    if(hasNoCanvas()) return NULL;
    return mCurrentCanvas->createImageBox(path);
}

SingleSound *CanvasWindow::createSoundForPath(const QString &path) {
    if(hasNoCanvas()) return NULL;
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
    mAudioFormat.setSampleRate(SAMPLERATE);
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
    mAudioIOOutput = NULL;
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
        MainWindow::getInstance()->blockUndoRedo();
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
        MainWindow::getInstance()->unblockUndoRedo();

        if(importedBox != NULL) {
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
                                                   "*.mp4 *.mov *.avi "
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
