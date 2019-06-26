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
#include "CacheHandlers/soundcachecontainer.h"
#include "svgimporter.h"
#include "filesourcescache.h"
#include <QFileDialog>
#include "videoencoder.h"
#include "usagewidget.h"
#include "memorychecker.h"

CanvasWindow::CanvasWindow(Document &document,
                           QWidget * const parent) :
    GLWindow(parent), mDocument(document), mActions(document.fActions) {
    //setAttribute(Qt::WA_OpaquePaintEvent, true);
    connect(&mDocument, &Document::canvasModeSet,
            this, &CanvasWindow::setCanvasMode);

    mPreviewFPSTimer = new QTimer(this);
    connect(mPreviewFPSTimer, &QTimer::timeout,
            this, &CanvasWindow::nextPreviewFrame);

    initializeAudio();

    this->setAcceptDrops(true);

    this->setMouseTracking(true);


    const auto vidEmitter = VideoEncoder::getVideoEncoderEmitter();
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
    setCurrentCanvas(nullptr);
}

Canvas *CanvasWindow::getCurrentCanvas() {
    return mCurrentCanvas;
}

void CanvasWindow::setCurrentCanvas(const int id) {
    if(id < 0 || id >= mDocument.fScenes.count()) {
        setCurrentCanvas(nullptr);
    } else {
        setCurrentCanvas(mDocument.fScenes.at(id).get());
    }
}

void CanvasWindow::setCurrentCanvas(Canvas * const canvas) {
    if(mCurrentCanvas) {
        mCurrentCanvas->setIsCurrentCanvas(false);
        disconnect(mCurrentCanvas, nullptr, this, nullptr);
        mDocument.removeVisibleScene(mCurrentCanvas);
    }
    mCurrentCanvas = canvas;
    if(hasFocus()) mDocument.setActiveScene(mCurrentCanvas);
    if(mCurrentCanvas) {
        mDocument.addVisibleScene(mCurrentCanvas);
        mCurrentSoundComposition = mCurrentCanvas->getSoundComposition();
        mCurrentCanvas->setIsCurrentCanvas(true);
        setCanvasMode(mCurrentCanvas->getCurrentCanvasMode());

        emit changeCanvasFrameRange(canvas->getFrameRange());
        changeCurrentFrameAction(getCurrentFrame());
        connect(mCurrentCanvas, &Canvas::requestCanvasMode,
                this, &CanvasWindow::setCanvasMode);
        connect(mCurrentCanvas, &Canvas::newerState,
                this, qOverload<>(&CanvasWindow::update));
        MainWindow::getInstance()->setCurrentUndoRedoStack(
                    mCurrentCanvas->getUndoRedoStack());
//        connect(mCurrentCanvas, &Canvas::prp_absFrameRangeChanged,
//                this, [this](const FrameRange& range) {
//            const int currFrame = mCurrentCanvas->anim_getCurrentAbsFrame();
//            if(range.inRange(currFrame)) update();
//        });
    } else {
        mCurrentSoundComposition = nullptr;
        MainWindow::getInstance()->setCurrentUndoRedoStack(nullptr);
    }

//    if(hasNoCanvas()) openWelcomeDialog();
//    else {
//        closeWelcomeDialog();
//        requestFitCanvasToSize();
//    }
    if(mCurrentCanvas) fitCanvasToSize();
    update();
}

void CanvasWindow::updatePaintModeCursor() {
    mValidPaintTarget = mCurrentCanvas && mCurrentCanvas->hasValidPaintTarget();
    if(mValidPaintTarget) {
        setCursor(QCursor(QPixmap(":/cursors/cursor_crosshair_precise_open.png")));
    } else {
        setCursor(QCursor(QPixmap(":/cursors/cursor_crosshair_open.png")));
    }
}

void CanvasWindow::setCanvasMode(const CanvasMode mode) {
    if(mode == MOVE_BOX) {
        setCursor(QCursor(Qt::ArrowCursor) );
    } else if(mode == MOVE_POINT) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-node.xpm"), 0, 0) );
    } else if(mode == PICK_PAINT_SETTINGS) {
        setCursor(QCursor(QPixmap(":/cursors/cursor_color_picker.png"), 2, 20) );
    } else if(mode == ADD_CIRCLE) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-ellipse.xpm"), 4, 4) );
    } else if(mode == ADD_RECTANGLE ||
              mode == ADD_PARTICLE_BOX) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-rect.xpm"), 4, 4) );
    } else if(mode == ADD_TEXT) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-text.xpm"), 4, 4) );
    } else if(mode == PAINT_MODE) {
        updatePaintModeCursor();
    } else {
        setCursor(QCursor(QPixmap(":/cursors/cursor-pen.xpm"), 4, 4) );
    }
    MainWindow::getInstance()->updateCanvasModeButtonsChecked();
    if(!mCurrentCanvas) return;
    if(mMouseGrabber) {
        mCurrentCanvas->cancelCurrentTransform();
        releaseMouse();
    }
    update();
}

void CanvasWindow::queScheduledTasksAndUpdate() {
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
    update();
}

bool CanvasWindow::hasNoCanvas() {
    return !mCurrentCanvas;
}

void CanvasWindow::renameCurrentCanvas(const QString &newName) {
    if(!mCurrentCanvas) return;
    mCurrentCanvas->setName(newName);
}

#include "glhelpers.h"

void CanvasWindow::renderSk(SkCanvas * const canvas,
                            GrContext* const grContext) {
    if(mCurrentCanvas) {
        canvas->save();
        mCurrentCanvas->renderSk(canvas, grContext, rect(),
                                 mViewTransform, mMouseGrabber);
        canvas->restore();
    }
    if(hasFocus()) {
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(4);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(SkRect::MakeWH(width(), height()), paint);
    }
}

void CanvasWindow::tabletEvent(QTabletEvent *e) {
    if(!mCurrentCanvas) return;
    if(mDocument.fCanvasMode != PAINT_MODE) return;
    const QPoint globalPos = mapToGlobal(QPoint(0, 0));
    const qreal x = e->hiResGlobalX() - globalPos.x();
    const qreal y = e->hiResGlobalY() - globalPos.y();
    mCurrentCanvas->tabletEvent(e, QPointF(x, y));
    if(!mValidPaintTarget) updatePaintModeCursor();
    update();
}

void CanvasWindow::mousePressEvent(QMouseEvent *event) {
    KFT_setFocus();
    if(!mCurrentCanvas || mBlockInput) return;
    if(mMouseGrabber && event->button() == Qt::LeftButton) return;
    const auto pos = mapToCanvasCoord(event->pos());
    mCurrentCanvas->mousePressEvent(
                MouseEvent(pos, pos, pos, mMouseGrabber,
                           mViewTransform.m11(), event,
                           [this]() { releaseMouse(); },
                           [this]() { grabMouse(); },
                           this));
    queScheduledTasksAndUpdate();
    mPrevMousePos = pos;
    if(event->button() == Qt::LeftButton) {
        mPrevPressPos = pos;
        if(mDocument.fCanvasMode == PAINT_MODE && !mValidPaintTarget)
            updatePaintModeCursor();
    }
}

void CanvasWindow::mouseReleaseEvent(QMouseEvent *event) {
    if(!mCurrentCanvas || mBlockInput) return;
    const auto pos = mapToCanvasCoord(event->pos());
    mCurrentCanvas->mouseReleaseEvent(
                MouseEvent(pos, mPrevMousePos, mPrevPressPos,
                           mMouseGrabber, mViewTransform.m11(),
                           event, [this]() { releaseMouse(); },
                           [this]() { grabMouse(); },
                           this));
    queScheduledTasksAndUpdate();
}

void CanvasWindow::mouseMoveEvent(QMouseEvent *event) {
    if(!mCurrentCanvas || mBlockInput) return;
    auto pos = mapToCanvasCoord(event->pos());
    if(event->buttons() & Qt::MiddleButton) {
        translateView(pos - mPrevMousePos);
        pos = mPrevMousePos;
    }
    mCurrentCanvas->mouseMoveEvent(
                MouseEvent(pos, mPrevMousePos, mPrevPressPos,
                           mMouseGrabber, mViewTransform.m11(),
                           event, [this]() { releaseMouse(); },
                           [this]() { grabMouse(); },
                           this));

    if(mDocument.fCanvasMode == PAINT_MODE) update();
    else queScheduledTasksAndUpdate();
    mPrevMousePos = pos;
}

void CanvasWindow::wheelEvent(QWheelEvent *event) {
    if(!mCurrentCanvas) return;
    if(event->delta() > 0) {
        zoomView(1.1, event->posF());
    } else {
        zoomView(0.9, event->posF());
    }
    update();
}

void CanvasWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    if(!mCurrentCanvas || mBlockInput) return;
    const auto pos = mapToCanvasCoord(event->pos());
    mCurrentCanvas->mouseDoubleClickEvent(
                MouseEvent(pos, mPrevMousePos, mPrevPressPos,
                           mMouseGrabber, mViewTransform.m11(),
                           event, [this]() { releaseMouse(); },
                           [this]() { grabMouse(); },
                           this));
    queScheduledTasksAndUpdate();
}

void CanvasWindow::openSettingsWindowForCurrentCanvas() {
    if(!mCurrentCanvas) return;
    const auto dialog = new CanvasSettingsDialog(mCurrentCanvas, this);
    connect(dialog, &QDialog::accepted, this, [dialog, this]() {
        dialog->applySettingsToCanvas(mCurrentCanvas.data());
        setCurrentCanvas(mCurrentCanvas.data());
        dialog->close();
    });
    dialog->show();
}

bool CanvasWindow::handleCutCopyPasteKeyPress(QKeyEvent *event) {
    if(event->modifiers() & Qt::ControlModifier &&
            event->key() == Qt::Key_V) {
        if(event->isAutoRepeat()) return false;
        mActions.pasteAction();
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_C) {
        if(event->isAutoRepeat()) return false;
        mActions.copyAction();
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_D) {
        if(event->isAutoRepeat()) return false;
        mActions.duplicateAction();
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_X) {
        if(event->isAutoRepeat()) return false;
        mActions.cutAction();
    } else if(event->key() == Qt::Key_Delete) {
        mActions.deleteAction();
    } else {
        return false;
    }
    return true;
}

bool CanvasWindow::handleTransformationKeyPress(QKeyEvent *event) {
    const int key = event->key();
    const bool keypad = event->modifiers() & Qt::KeypadModifier;
    if(key == Qt::Key_0 && keypad) {
        fitCanvasToSize();
    } else if(key == Qt::Key_1 && keypad) {
        resetTransormation();
    } else if(key == Qt::Key_Minus || key == Qt::Key_Plus) {
       if(mCurrentCanvas->isPreviewingOrRendering()) return false;
       const auto relPos = mapFromGlobal(QCursor::pos());
       if(event->key() == Qt::Key_Plus) zoomView(1.2, relPos);
       else zoomView(0.8, relPos);
    } else return false;
    update();
    return true;
}

bool CanvasWindow::handleZValueKeyPress(QKeyEvent *event) {
    if(event->key() == Qt::Key_PageUp) {
       mCurrentCanvas->raiseSelectedBoxes();
    } else if(event->key() == Qt::Key_PageDown) {
       mCurrentCanvas->lowerSelectedBoxes();
    } else if(event->key() == Qt::Key_End) {
       mCurrentCanvas->lowerSelectedBoxesToBottom();
    } else if(event->key() == Qt::Key_Home) {
       mCurrentCanvas->raiseSelectedBoxesToTop();
    } else return false;
    return true;
}

bool CanvasWindow::handleParentChangeKeyPress(QKeyEvent *event) {
    if(event->modifiers() & Qt::ControlModifier &&
       event->key() == Qt::Key_P) {
        mCurrentCanvas->setParentToLastSelected();
    } else if(event->modifiers() & Qt::AltModifier &&
              event->key() == Qt::Key_P) {
        mCurrentCanvas->clearParentForSelected();
    } else {
        return false;
    }
    return true;
}

bool CanvasWindow::handleGroupChangeKeyPress(QKeyEvent *event) {
    if(event->modifiers() & Qt::ControlModifier &&
       event->key() == Qt::Key_G) {
       if(event->modifiers() & Qt::ShiftModifier) {
           mActions.ungroupSelectedBoxes();
       } else {
           mActions.groupSelectedBoxes();
       }
    } else {
        return false;
    }
    return true;
}

bool CanvasWindow::handleResetTransformKeyPress(QKeyEvent *event) {
    bool altPressed = event->modifiers() & Qt::AltModifier;
    if(event->key() == Qt::Key_G && altPressed) {
        mCurrentCanvas->resetSelectedTranslation();
    } else if(event->key() == Qt::Key_S && altPressed) {
        mCurrentCanvas->resetSelectedScale();
    } else if(event->key() == Qt::Key_R && altPressed) {
        mCurrentCanvas->resetSelectedRotation();
    } else {
        return false;
    }
    return true;
}

bool CanvasWindow::handleRevertPathKeyPress(QKeyEvent *event) {
    if(event->modifiers() & Qt::ControlModifier &&
       (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)) {
       if(event->modifiers() & Qt::ShiftModifier) {
           mCurrentCanvas->revertAllPointsForAllKeys();
       } else {
           mCurrentCanvas->revertAllPoints();
       }
    } else {
        return false;
    }
    return true;
}

bool CanvasWindow::handleStartTransformKeyPress(const KeyEvent& e) {
    if(mMouseGrabber) return false;
    if(e.fKey == Qt::Key_R) {
        return mCurrentCanvas->startRotatingAction(e);
    } else if(e.fKey == Qt::Key_S) {
        return mCurrentCanvas->startScalingAction(e);
    } else if(e.fKey == Qt::Key_G) {
        return mCurrentCanvas->startMovingAction(e);
    } else return false;
}

bool CanvasWindow::handleSelectAllKeyPress(QKeyEvent* event) {
    if(event->key() == Qt::Key_A && !isMouseGrabber()) {
        bool altPressed = event->modifiers() & Qt::AltModifier;
        auto currentMode = mCurrentCanvas->getCurrentCanvasMode();
        if(currentMode == MOVE_BOX) {
            if(altPressed) {
               mCurrentCanvas->deselectAllBoxesAction();
           } else {
               mCurrentCanvas->selectAllBoxesAction();
           }
        } else if(currentMode == MOVE_POINT) {
            if(altPressed) {
                mCurrentCanvas->clearPointsSelection();
            } else {
                mCurrentCanvas->selectAllPointsAction();
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

bool CanvasWindow::handleShiftKeysKeyPress(QKeyEvent* event) {
    if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_Right) {
        if(event->modifiers() & Qt::ShiftModifier) {
            mCurrentCanvas->shiftAllPointsForAllKeys(1);
        } else {
            mCurrentCanvas->shiftAllPoints(1);
        }
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_Left) {
        if(event->modifiers() & Qt::ShiftModifier) {
            mCurrentCanvas->shiftAllPointsForAllKeys(-1);
        } else {
            mCurrentCanvas->shiftAllPoints(-1);
        }
    } else {
        return false;
    }
    return true;
}
#include <QApplication>
bool CanvasWindow::KFT_handleKeyEventForTarget(QKeyEvent *event) {
    if(hasNoCanvas()) return false;
    if(mCurrentCanvas->isPreviewingOrRendering()) return false;
    const QPoint globalPos = QCursor::pos();
    const auto pos = mapToCanvasCoord(mapFromGlobal(globalPos));
    const KeyEvent e(pos, mPrevMousePos, mPrevPressPos, mMouseGrabber,
                     mViewTransform.m11(), globalPos,
                     QApplication::mouseButtons(), event,
                     [this]() { releaseMouse(); },
                     [this]() { grabMouse(); },
                     this);
    if(isMouseGrabber()) {
        if(mCurrentCanvas->handleTransormationInputKeyEvent(e)) return true;
    }
    if(mCurrentCanvas->handlePaintModeKeyPress(e)) return true;
    if(handleCutCopyPasteKeyPress(event)) return true;
    if(handleTransformationKeyPress(event)) return true;
    if(handleZValueKeyPress(event)) return true;
    if(handleParentChangeKeyPress(event)) return true;
    if(handleGroupChangeKeyPress(event)) return true;
    if(handleResetTransformKeyPress(event)) return true;
    if(handleRevertPathKeyPress(event)) return true;
    if(handleStartTransformKeyPress(e)) {
        mPrevPressPos = pos;
        mPrevMousePos = pos;
        return true;
    } if(handleSelectAllKeyPress(event)) return true;
    if(handleShiftKeysKeyPress(event)) return true;

    if(e.fKey == Qt::Key_I && !isMouseGrabber()) {
        mActions.invertSelectionAction();
    } else if(e.fKey == Qt::Key_W) {
        mDocument.incBrushRadius();
    } else if(e.fKey == Qt::Key_Q) {
        mDocument.decBrushRadius();
    } else return false;

    return true;
}

#include "welcomedialog.h"
void CanvasWindow::openWelcomeDialog() {
    return;
    if(mWelcomeDialog) return;
    const auto mWindow = MainWindow::getInstance();
    mWelcomeDialog = new WelcomeDialog(mWindow->getRecentFiles(),
                                       [this]() { CanvasSettingsDialog::sNewCanvasDialog(mDocument, this); },
                                       []() { MainWindow::getInstance()->openFile(); },
                                       [](QString path) { MainWindow::getInstance()->openFile(path); },
                                       mWindow);
    mWelcomeDialog->resize(size());
    mWindow->takeCentralWidget();
    mWindow->setCentralWidget(mWelcomeDialog);
}

void CanvasWindow::closeWelcomeDialog() {
    return;
    if(!mWelcomeDialog) return;

    const auto mWindow = MainWindow::getInstance();
    resize(mWelcomeDialog->size());
    mWelcomeDialog = nullptr;
    mWindow->setCentralWidget(this);
}

void CanvasWindow::changeCurrentFrameAction(const int frame) {
    emit changeCurrentFrame(frame);
    if(mCurrentCanvas) mCurrentCanvas->anim_setAbsFrame(frame);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::setResolutionFraction(const qreal percent) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setResolutionFraction(percent);
    mCurrentCanvas->prp_afterWholeInfluenceRangeChanged();
    mCurrentCanvas->updateAllBoxes(Animator::USER_CHANGE);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::updatePivotIfNeeded() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->updatePivotIfNeeded();
}

void CanvasWindow::schedulePivotUpdate() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->schedulePivotUpdate();
}

ContainerBox *CanvasWindow::getCurrentGroup() {
    if(hasNoCanvas()) return nullptr;
    return mCurrentCanvas->getCurrentGroup();
}

void CanvasWindow::renderFromSettings(RenderInstanceSettings * const settings) {
    VideoEncoder::sStartEncoding(settings);
    if(VideoEncoder::sEncodingSuccessfulyStarted()) {
        mSavedCurrentFrame = getCurrentFrame();
        mSavedResolutionFraction = mCurrentCanvas->getResolutionFraction();

        mCurrentRenderSettings = settings;
        const RenderSettings &renderSettings = settings->getRenderSettings();
        Canvas * const canvas = settings->getTargetCanvas();
        setCurrentCanvas(canvas);
        changeCurrentFrameAction(renderSettings.fMinFrame);

        const qreal resolutionFraction = renderSettings.fResolution;
        mMinRenderFrame = renderSettings.fMinFrame;
        mMaxRenderFrame = renderSettings.fMaxFrame;

        const auto nextFrameFunc = [this]() {
            nextSaveOutputFrame();
        };
        TaskScheduler::sSetFreeThreadsForCPUTasksAvailableFunc(nextFrameFunc);
        TaskScheduler::sSetAllTasksFinishedFunc(nextFrameFunc);

        fitCanvasToSize();
        if(!isZero6Dec(mSavedResolutionFraction - resolutionFraction)) {
            mCurrentCanvas->setResolutionFraction(resolutionFraction);
        }

        mCurrentRenderFrame = renderSettings.fMinFrame;
        mCurrRenderRange = {mCurrentRenderFrame, mCurrentRenderFrame};
        mCurrentCanvas->setCurrentRenderRange(mCurrRenderRange);

        mCurrentEncodeFrame = mCurrentRenderFrame;
        mFirstEncodeSoundSecond = qRound(mCurrentRenderFrame/mCurrentCanvas->getFps());
        mCurrentEncodeSoundSecond = mFirstEncodeSoundSecond;
        mCurrentSoundComposition->startBlockingAtFrame(mCurrentRenderFrame);
        mCurrentSoundComposition->scheduleFrameRange({mCurrentRenderFrame,
                                                      mCurrentRenderFrame});
        mCurrentCanvas->anim_setAbsFrame(mCurrentRenderFrame);
        mCurrentCanvas->setOutputRendering(true);
        if(TaskScheduler::sAllQuedCPUTasksFinished()) {
            nextSaveOutputFrame();
        }
    }
}

void CanvasWindow::nextCurrentRenderFrame() {
    auto& cacheHandler = mCurrentCanvas->getCacheHandler();
    int newCurrentRenderFrame = cacheHandler.
            firstEmptyFrameAtOrAfter(mCurrentRenderFrame);
    if(newCurrentRenderFrame - mCurrentRenderFrame > 1) {
        const int minBlock = mCurrentRenderFrame + 1;
        const int maxBlock = newCurrentRenderFrame - 1;
        cacheHandler.blockConts({minBlock, maxBlock}, true);
    }
    const bool allDone = newCurrentRenderFrame > mMaxRenderFrame;
    newCurrentRenderFrame = qMin(mMaxRenderFrame, newCurrentRenderFrame);
    const FrameRange newSoundRange = {mCurrentRenderFrame, newCurrentRenderFrame};
    mCurrentSoundComposition->scheduleFrameRange(newSoundRange);
    mCurrentSoundComposition->blockUpToFrame(newCurrentRenderFrame);

    mCurrentRenderFrame = newCurrentRenderFrame;
    mCurrRenderRange.fMax = mCurrentRenderFrame;
    mCurrentCanvas->setCurrentRenderRange(mCurrRenderRange);
    if(!allDone) changeCurrentFrameAction(mCurrentRenderFrame);
}

void CanvasWindow::renderPreview() {
    if(hasNoCanvas()) return;
    const auto nextFrameFunc = [this]() {
        nextPreviewRenderFrame();
    };
    TaskScheduler::sSetFreeThreadsForCPUTasksAvailableFunc(nextFrameFunc);
    TaskScheduler::sSetAllTasksFinishedFunc(nextFrameFunc);

    mSavedCurrentFrame = getCurrentFrame();    
    mCurrentRenderFrame = mSavedCurrentFrame;
    mCurrRenderRange = {mCurrentRenderFrame, mCurrentRenderFrame};
    mCurrentCanvas->setCurrentRenderRange(mCurrRenderRange);
    mCurrentSoundComposition->startBlockingAtFrame(mCurrentRenderFrame);

    mMaxRenderFrame = getMaxFrame();
    setRenderingPreview(true);

    MainWindow::getInstance()->previewBeingRendered();
    if(TaskScheduler::sAllQuedCPUTasksFinished()) {
        nextPreviewRenderFrame();
    }
}

void CanvasWindow::interruptPreview() {
    if(mRenderingPreview) interruptPreviewRendering();
    else if(mPreviewing) stopPreview();
}

void CanvasWindow::outOfMemory() {
    if(mRenderingPreview) {
        TaskScheduler::sClearTasks();
        playPreview();
    }
}

void CanvasWindow::setRenderingPreview(const bool bT) {
    mRenderingPreview = bT;
    mCurrentCanvas->setRenderingPreview(bT);
}

void CanvasWindow::setPreviewing(const bool bT) {
    mPreviewing = bT;
    mCurrentCanvas->setPreviewing(bT);
}

void CanvasWindow::interruptPreviewRendering() {
    setRenderingPreview(false);
    TaskScheduler::sClearAllFinishedFuncs();
    clearPreview();
    auto& cacheHandler = mCurrentCanvas->getCacheHandler();
    cacheHandler.blockConts({mSavedCurrentFrame + 1, mMaxRenderFrame}, false);
    changeCurrentFrameAction(mSavedCurrentFrame);
    MainWindow::getInstance()->previewFinished();
}

void CanvasWindow::interruptOutputRendering() {
    mCurrentCanvas->setOutputRendering(false);
    TaskScheduler::sClearAllFinishedFuncs();
    clearPreview();
    changeCurrentFrameAction(mSavedCurrentFrame);
}

void CanvasWindow::stopPreview() {
    setPreviewing(false);
    auto& cacheHandler = mCurrentCanvas->getCacheHandler();
    cacheHandler.blockConts({mSavedCurrentFrame + 1, mMaxRenderFrame}, false);
    changeCurrentFrameAction(mSavedCurrentFrame);
    mCurrentCanvas->setCurrentPreviewContainer(mSavedCurrentFrame);
    mPreviewFPSTimer->stop();
    stopAudio();
    update();
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

void CanvasWindow::playPreviewAfterAllTasksCompleted() {
    if(mRenderingPreview) {
        if(TaskScheduler::sAllTasksFinished()) {
            playPreview();
        } else {
            const auto allFinishedFunc = [this]() {
                playPreview();
            };
            TaskScheduler::sSetAllTasksFinishedFunc(allFinishedFunc);
        }
    }
}

void CanvasWindow::playPreview() {
    if(hasNoCanvas()) return;
    //changeCurrentFrameAction(mSavedCurrentFrame);
    TaskScheduler::sClearAllFinishedFuncs();
    const int minPreviewFrame = mSavedCurrentFrame;
    const int maxPreviewFrame = qMin(mMaxRenderFrame, mCurrentRenderFrame);
    if(minPreviewFrame >= maxPreviewFrame) return;
    mMaxPreviewFrame = maxPreviewFrame;
    mCurrentPreviewFrame = minPreviewFrame;
    mCurrentCanvas->setCurrentPreviewContainer(mCurrentPreviewFrame);
    mCurrentCanvas->setPreviewing(true);

    setRenderingPreview(false);
    setPreviewing(true);

    startAudio();
    const int mSecInterval = qRound(1000/mCurrentCanvas->getFps());
    mPreviewFPSTimer->setInterval(mSecInterval);
    mPreviewFPSTimer->start();
    MainWindow::getInstance()->previewBeingPlayed();
    update();
}

void CanvasWindow::nextPreviewRenderFrame() {
    if(!mRenderingPreview) return;
    if(mCurrentRenderFrame >= mMaxRenderFrame) {
        playPreviewAfterAllTasksCompleted();
    } else {
        nextCurrentRenderFrame();
        if(TaskScheduler::sAllTasksFinished()) {
            nextPreviewRenderFrame();
        }
    }
}

void CanvasWindow::clearPreview() {
    MainWindow::getInstance()->previewFinished();
    stopPreview();
}

void CanvasWindow::nextPreviewFrame() {
    if(hasNoCanvas()) return;
    mCurrentPreviewFrame++;
    if(mCurrentPreviewFrame > mMaxPreviewFrame) {
        clearPreview();
    } else {
        mCurrentCanvas->setCurrentPreviewContainer(
                    mCurrentPreviewFrame);
        emit changeCurrentFrame(mCurrentPreviewFrame);
    }
    update();
}

void CanvasWindow::nextSaveOutputFrame() {
    const auto& sCacheHandler = mCurrentCanvas->getSoundComposition()->getCacheHandler();
    const qreal fps = mCurrentCanvas->getFps();
    const int maxSec = qCeil(mMaxRenderFrame/fps);
    while(mCurrentEncodeSoundSecond <= maxSec) {
        const auto cont = sCacheHandler.atFrame(mCurrentEncodeSoundSecond);
        if(!cont) break;
        const auto sCont = GetAsSPtr(cont, SoundCacheContainer);
        const auto samples = sCont->getSamples();
        if(mCurrentEncodeSoundSecond == mFirstEncodeSoundSecond) {
            const int minSample = qRound(mMinRenderFrame*SOUND_SAMPLERATE/fps);
            const int max = samples->fSampleRange.fMax;
            VideoEncoder::sAddCacheContainerToEncoder(
                        SPtrCreate(Samples)(samples->mid({minSample, max})));
        } else {
            VideoEncoder::sAddCacheContainerToEncoder(
                        SPtrCreate(Samples)(samples));
        }
        sCont->setBlocked(false);
        mCurrentEncodeSoundSecond++;
    }

    const auto& cacheHandler = mCurrentCanvas->getCacheHandler();
    while(mCurrentEncodeFrame <= mMaxRenderFrame) {
        const auto cont = cacheHandler.atFrame(mCurrentEncodeFrame);
        if(!cont) break;
        VideoEncoder::sAddCacheContainerToEncoder(
                    GetAsSPtr(cont, ImageCacheContainer));
        mCurrentEncodeFrame = cont->getRangeMax() + 1;
    }

    //mCurrentCanvas->renderCurrentFrameToOutput(*mCurrentRenderSettings);
    if(mCurrentRenderFrame >= mMaxRenderFrame) {
        queScheduledTasksAndUpdate();
        if(TaskScheduler::sAllTasksFinished()) {
            TaskScheduler::sClearAllFinishedFuncs();
            mCurrentRenderSettings = nullptr;
            mCurrentCanvas->setOutputRendering(false);
            changeCurrentFrameAction(mSavedCurrentFrame);
            if(qAbs(mSavedResolutionFraction -
                    mCurrentCanvas->getResolutionFraction()) > 0.1) {
                mCurrentCanvas->setResolutionFraction(mSavedResolutionFraction);
            }
            mCurrentSoundComposition->unblockAll();
            VideoEncoder::sFinishEncoding();
        }
    } else {
        mCurrentRenderSettings->setCurrentRenderFrame(mCurrentRenderFrame);
        nextCurrentRenderFrame();
        if(TaskScheduler::sAllTasksFinished()) {
            nextSaveOutputFrame();
        }
    }
}

void CanvasWindow::clearAll() {
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

void CanvasWindow::setLocalPivot(const bool bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setLocalPivot(bT);
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
        //RuntimeThrow("Default format not supported - trying to use nearest");
        mAudioFormat = info.nearestFormat(mAudioFormat);
    }

    mAudioOutput = new QAudioOutput(mAudioDevice, mAudioFormat, this);
}

void CanvasWindow::startAudio() {
    mCurrentSoundComposition->start(mCurrentPreviewFrame);
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

void CanvasWindow::volumeChanged(const int value) {
    if(mAudioOutput) mAudioOutput->setVolume(qreal(value/100.));
}

void CanvasWindow::pushTimerExpired() {
    if(mAudioOutput && mAudioOutput->state() != QAudio::StoppedState) {
        int chunks = mAudioOutput->bytesFree()/mAudioOutput->periodSize();
        while(chunks) {
           const qint64 len = mCurrentSoundComposition->read(
                                                mAudioBuffer.data(),
                                                mAudioOutput->periodSize());
           if(len) mAudioIOOutput->write(mAudioBuffer.data(), len);
           if(len != mAudioOutput->periodSize()) break;
           --chunks;
        }
    }
}

void CanvasWindow::dropEvent(QDropEvent *event) {
    const QMimeData* mimeData = event->mimeData();

    if(mimeData->hasUrls()) {
        event->acceptProposedAction();
        const QList<QUrl> urlList = mimeData->urls();
        for(int i = 0; i < urlList.size() && i < 32; i++) {
            try {
                const QPointF pos = mapToCanvasCoord(event->posF());
                importFile(urlList.at(i).toLocalFile(), pos);
            } catch(const std::exception& e) {
                gPrintExceptionCritical(e);
            }
        }
    }
}

void CanvasWindow::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CanvasWindow::dragMoveEvent(QDragMoveEvent *event) {
    if(event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CanvasWindow::importFile(const QString &path,
                              const QPointF &relDropPos) {
    if(hasNoCanvas()) return;

    const QFile file(path);
    if(!file.exists())
        RuntimeThrow("File " + path + " does not exit.");

    const QString extension = path.split(".").last();
    if(isSoundExt(extension)) {
        createSoundForPath(path);
    } else {
        qsptr<BoundingBox> importedBox;
        mCurrentCanvas->blockUndoRedo();
        if(isVectorExt(extension)) {
            importedBox = loadSVGFile(path);
        } else if(isImageExt(extension)) {
            const auto imgBox = SPtrCreate(ImageBox)();
            importedBox = imgBox;
            imgBox->setFilePath(path);
        } else if(isVideoExt(extension)) {
            const auto vidBox = SPtrCreate(VideoBox)();
            importedBox = vidBox;
            vidBox->setFilePath(path);
        } else if(isEvExt(extension)) {
            MainWindow::getInstance()->loadEVFile(path);
        } else {
            mCurrentCanvas->unblockUndoRedo();
            RuntimeThrow("Unrecognized file extension " + path + ".");
        }
        mCurrentCanvas->unblockUndoRedo();

        if(importedBox) {
            importedBox->planCenterPivotPosition();
            mCurrentCanvas->getCurrentGroup()->addContainedBox(importedBox);
            importedBox->moveByAbs(relDropPos);
        }
    }
    queScheduledTasksAndUpdate();
}

void CanvasWindow::grabMouse() {
    mMouseGrabber = true;
#ifndef QT_DEBUG
    QWidget::grabMouse();
#endif
}

void CanvasWindow::releaseMouse() {
    mMouseGrabber = false;
#ifndef QT_DEBUG
    QWidget::releaseMouse();
#endif
}

bool CanvasWindow::isMouseGrabber() {
    return mMouseGrabber;
}

void CanvasWindow::importFile() {
    MainWindow::getInstance()->disableEventFilter();
    QStringList importPaths = QFileDialog::getOpenFileNames(
                                            MainWindow::getInstance(),
                                            "Import File", "",
                                            "Files (*.ev *.svg "
                                                   "*.mp4 *.mov *.avi *.mkv *.m4v "
                                                   "*.png *.jpg "
                                                   "*.wav *.mp3)");
    MainWindow::getInstance()->enableEventFilter();
    if(!importPaths.isEmpty()) {
        for(const QString &path : importPaths) {
            if(path.isEmpty()) continue;
            try {
                importFile(path);
            } catch(const std::exception& e) {
                gPrintExceptionCritical(e);
            }
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

void CanvasWindow::moveDurationRectForAllSelected(const int dFrame) {
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

void CanvasWindow::moveMinFrameForAllSelected(const int dFrame) {
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

void CanvasWindow::moveMaxFrameForAllSelected(const int dFrame) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->moveMaxFrameForAllSelected(dFrame);
}
