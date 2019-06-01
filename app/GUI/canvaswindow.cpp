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
    connect(mPreviewFPSTimer, &QTimer::timeout,
            this, &CanvasWindow::nextPreviewFrame);

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

Canvas *CanvasWindow::getCurrentCanvas() {
    return mCurrentCanvas;
}

void CanvasWindow::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction *abstraction,
        const UpdateFuncs &updateFuncs,
        const int& visiblePartWidgetId) {
    for(const auto& child : mCanvasList) {
        auto abs = child->SWT_getOrCreateAbstractionForWidget(updateFuncs,
                                                      visiblePartWidgetId);
        abstraction->addChildAbstraction(abs);
    }
}

void CanvasWindow::setCurrentCanvas(int id) {
    if(id < 0 || id >= mCanvasList.count()) {
        setCurrentCanvas(nullptr);
    } else {
        setCurrentCanvas(mCanvasList.at(id).get());
    }
}

void CanvasWindow::setCurrentCanvas(Canvas * const canvas) {
    TaskScheduler::sSetCurrentCanvas(canvas);
    if(mCurrentCanvas) mCurrentCanvas->setIsCurrentCanvas(false);

    if(canvas) {
        mCurrentCanvas = canvas;
        mCurrentSoundComposition = mCurrentCanvas->getSoundComposition();
        mCurrentCanvas->setIsCurrentCanvas(true);
        setCanvasMode(mCurrentCanvas->getCurrentCanvasMode());

        emit changeCanvasFrameRange(0, getMaxFrame());
        changeCurrentFrameAction(getCurrentFrame());
    } else {
        mCurrentSoundComposition = nullptr;
        mCurrentCanvas.clear();
    }
    ContainerBox *currentGroup;
    if(mCurrentCanvas) {
        MainWindow::getInstance()->setCurrentUndoRedoStack(
                    mCurrentCanvas->getUndoRedoStack());
        currentGroup = mCurrentCanvas->getCurrentBoxesGroup();
    } else {
        MainWindow::getInstance()->setCurrentUndoRedoStack(nullptr);
        currentGroup = nullptr;
    }
    mWindowSWTTarget->SWT_scheduleWidgetsContentUpdateWithTarget(
                currentGroup,
                SWT_TARGET_CURRENT_GROUP);
    mWindowSWTTarget->SWT_scheduleWidgetsContentUpdateWithTarget(
                mCurrentCanvas.data(),
                SWT_TARGET_CURRENT_CANVAS);
    MainWindow::getInstance()->updateSettingsForCurrentCanvas();
    if(hasNoCanvas()) openWelcomeDialog();
    else {
        closeWelcomeDialog();
        mCurrentCanvas->fitCanvasToSize();
    }
    queScheduledTasksAndUpdate();
}

void CanvasWindow::addCanvasToList(const qsptr<Canvas>& canvas) {
    mCanvasList << canvas;
    mWindowSWTTarget->SWT_addChildAbstractionForTargetToAll(canvas.get());
}

void CanvasWindow::removeCanvas(const int &id) {
    const auto canvas = mCanvasList.takeAt(id);
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
        setCursor(QCursor(Qt::ArrowCursor));
        return;
    }

    if(mode == MOVE_BOX) {
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
    requestUpdate();
    MainWindow::getInstance()->updateCanvasModeButtonsChecked();
}

void CanvasWindow::queScheduledTasksAndUpdate() {
    MainWindow::getInstance()->queScheduledTasksAndUpdate();
}

void CanvasWindow::setMovePathMode() {
    setCanvasMode(MOVE_BOX);
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
    return !mCurrentCanvas;
}

void CanvasWindow::renameCurrentCanvas(const QString &newName) {
    if(!mCurrentCanvas) return;
    renameCanvas(mCurrentCanvas.data(), newName);
}

#include "glhelpers.h"

void CanvasWindow::renderSk(SkCanvas * const canvas,
                            GrContext* const grContext) {
    canvas->clear(SK_ColorBLACK);
    if(mCurrentCanvas) mCurrentCanvas->renderSk(canvas, grContext);
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
    if(!mCurrentCanvas) return;
    mCurrentCanvas->mousePressEvent(event);
}

void CanvasWindow::mouseReleaseEvent(QMouseEvent *event) {
    if(!mCurrentCanvas) return;
    mCurrentCanvas->mouseReleaseEvent(event);
}

void CanvasWindow::mouseMoveEvent(QMouseEvent *event) {
    if(!mCurrentCanvas) return;
    mCurrentCanvas->mouseMoveEvent(event);
}

void CanvasWindow::wheelEvent(QWheelEvent *event) {
    if(!mCurrentCanvas) return;
    mCurrentCanvas->wheelEvent(event);
}

void CanvasWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    if(!mCurrentCanvas) return;
    mCurrentCanvas->mouseDoubleClickEvent(event);
}

void CanvasWindow::openSettingsWindowForCurrentCanvas() {
    if(hasNoCanvas()) return;
    const auto dialog =
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

bool CanvasWindow::handleCanvasModeChangeKeyPress(QKeyEvent *event) {
    if(event->key() == Qt::Key_F1) {
        setCanvasMode(CanvasMode::MOVE_BOX);
    } else if(event->key() == Qt::Key_F2) {
        setCanvasMode(CanvasMode::MOVE_POINT);
    } else if(event->key() == Qt::Key_F3) {
            //setCanvasMode(CanvasMode::ADD_POINT);
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
    } else {
        return false;
    }
    return true;
}

bool CanvasWindow::handleCutCopyPasteKeyPress(QKeyEvent *event) {
    if(event->modifiers() & Qt::ControlModifier &&
            event->key() == Qt::Key_V) {
        if(event->isAutoRepeat()) return false;
        pasteAction();
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_C) {
        if(event->isAutoRepeat()) return false;
        copyAction();
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_D) {
        if(event->isAutoRepeat()) return false;
        duplicateAction();
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_X) {
        if(event->isAutoRepeat()) return false;
        cutAction();
    } else if(event->key() == Qt::Key_Delete) {
        deleteAction();
    } else {
        return false;
    }
    return true;
}

bool CanvasWindow::handleTransformationKeyPress(QKeyEvent *event) {
    if(event->key() == Qt::Key_0 &&
              event->modifiers() & Qt::KeypadModifier) {
        mCurrentCanvas->fitCanvasToSize();
    } else if(event->key() == Qt::Key_1 &&
              event->modifiers() & Qt::KeypadModifier) {
        mCurrentCanvas->resetTransormation();
    } else if(event->key() == Qt::Key_Minus ||
             event->key() == Qt::Key_Plus) {
       if(mCurrentCanvas->isPreviewingOrRendering()) return false;
       auto relPos = mapFromGlobal(QCursor::pos());
       if(event->key() == Qt::Key_Plus) {
           mCurrentCanvas->zoomCanvas(1.2, relPos);
       } else {
           mCurrentCanvas->zoomCanvas(0.8, relPos);
       }
    } else {
        return false;
    }
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
    } else {
        return false;
    }
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
           ungroupSelectedBoxes();
       } else {
           groupSelectedBoxes();
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

bool CanvasWindow::handleStartTransformKeyPress(QKeyEvent* event) {
    auto relPos = mapFromGlobal(QCursor::pos());
    if(event->key() == Qt::Key_R && !isMouseGrabber()) {
        return mCurrentCanvas->startRotatingAction(relPos);
    } else if(event->key() == Qt::Key_S && !isMouseGrabber()) {
        return mCurrentCanvas->startScalingAction(relPos);
    } else if(event->key() == Qt::Key_G && !isMouseGrabber()) {
        return mCurrentCanvas->startMovingAction(relPos);
    } else {
        return false;
    }
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

bool CanvasWindow::KFT_handleKeyEventForTarget(QKeyEvent *event) {
    if(hasNoCanvas()) return false;
    if(mCurrentCanvas->isPreviewingOrRendering()) return false;
    if(isMouseGrabber()) {
        if(mCurrentCanvas->handleTransormationInputKeyEvent(event)) return true;
    }
    if(handleCanvasModeChangeKeyPress(event)) return true;
    if(handleCutCopyPasteKeyPress(event)) return true;
    if(handleTransformationKeyPress(event)) return true;
    if(handleZValueKeyPress(event)) return true;
    if(handleParentChangeKeyPress(event)) return true;
    if(handleGroupChangeKeyPress(event)) return true;
    if(handleResetTransformKeyPress(event)) return true;
    if(handleRevertPathKeyPress(event)) return true;
    if(handleStartTransformKeyPress(event)) return true;
    if(handleSelectAllKeyPress(event)) return true;
    if(handleShiftKeysKeyPress(event)) return true;

    if(event->key() == Qt::Key_I && !isMouseGrabber()) {
        invertSelectionAction();
    } else if(event->key() == Qt::Key_W) {
        mCurrentCanvas->incBrushRadius();
    } else if(event->key() == Qt::Key_Q) {
        mCurrentCanvas->decBrushRadius();
    } else return false;

    return true;
}

void CanvasWindow::raiseAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->raiseSelectedBoxes();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::lowerAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->lowerSelectedBoxes();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::raiseToTopAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->raiseSelectedBoxesToTop();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::lowerToBottomAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->lowerSelectedBoxesToBottom();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::objectsToPathAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->convertSelectedBoxesToPath();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::strokeToPathAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->convertSelectedPathStrokesToPath();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::rotate90CWAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->rotateSelectedBoxesStartAndFinish(90.);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::rotate90CCWAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->rotateSelectedBoxesStartAndFinish(-90.);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::flipHorizontalAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->flipSelectedBoxesHorizontally();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::flipVerticalAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->flipSelectedBoxesVertically();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::setCurrentBrush(const SimpleBrushWrapper * const brush) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setCurrentBrush(brush);
}

void CanvasWindow::pathsUnionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsUnion();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::pathsDifferenceAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsDifference();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::pathsIntersectionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsIntersection();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::pathsDivisionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsDivision();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::pathsExclusionAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsExclusion();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::pathsCombineAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsCombine();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::pathsBreakApartAction() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->selectedPathsBreakApart();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::setFontFamilyAndStyle(const QString& family,
                                         const QString& style) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFontFamilyAndStyle(family, style);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::setFontSize(const qreal& size) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFontSize(size);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::connectPointsSlot() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->connectPoints();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::disconnectPointsSlot() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->disconnectPoints();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::mergePointsSlot() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->mergePoints();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::makePointCtrlsSymmetric() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makePointCtrlsSymmetric();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::makePointCtrlsSmooth() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makePointCtrlsSmooth();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::makePointCtrlsCorner() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makePointCtrlsCorner();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::makeSegmentLine() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makeSegmentLine();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::makeSegmentCurve() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->makeSegmentCurve();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::startSelectedStrokeWidthTransform() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->startSelectedStrokeWidthTransform();
    queScheduledTasksAndUpdate();
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
    queScheduledTasksAndUpdate();
}

void CanvasWindow::ungroupSelectedBoxes() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->ungroupSelectedBoxes();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::startSelectedStrokeColorTransform() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->startSelectedStrokeColorTransform();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::startSelectedFillColorTransform() {
    if(hasNoCanvas()) return;
    mCurrentCanvas->startSelectedFillColorTransform();
    queScheduledTasksAndUpdate();
}

void CanvasWindow::strokeCapStyleChanged(const Qt::PenCapStyle &capStyle) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedCapStyle(capStyle);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::strokeJoinStyleChanged(const Qt::PenJoinStyle &joinStyle) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedJoinStyle(joinStyle);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::strokeBrushChanged(SimpleBrushWrapper * const brush) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeBrush(brush);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::strokeBrushWidthCurveChanged(
        const qCubicSegment1D& curve) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeBrushWidthCurve(curve);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::strokeBrushTimeCurveChanged(
        const qCubicSegment1D& curve) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeBrushTimeCurve(curve);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::strokeBrushSpacingCurveChanged(
        const qCubicSegment1D& curve) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeBrushSpacingCurve(curve);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::strokeBrushPressureCurveChanged(
        const qCubicSegment1D& curve) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeBrushPressureCurve(curve);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::strokeWidthChanged(const qreal &strokeWidth) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeWidth(strokeWidth);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::applyPaintSettingToSelected(const PaintSettingsApplier &setting) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->applyPaintSettingToSelected(setting);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::setSelectedFillColorMode(const ColorMode &mode) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedFillColorMode(mode);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::setSelectedStrokeColorMode(const ColorMode &mode) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setSelectedStrokeColorMode(mode);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::updateAfterFrameChanged(const int &currentFrame) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->anim_setAbsFrame(currentFrame);
}

void CanvasWindow::getDisplayedFillStrokeSettingsFromLastSelected(
        PaintSettingsAnimator*& fillSetings, OutlineSettingsAnimator*& strokeSettings) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->getDisplayedFillStrokeSettingsFromLastSelected(
                fillSetings, strokeSettings);
}
#include "welcomedialog.h"
void CanvasWindow::openWelcomeDialog() {
    if(mWelcomeDialog) return;
    const auto mWindow = MainWindow::getInstance();
    mWelcomeDialog = new WelcomeDialog(mWindow->getRecentFiles(),
                                       []() { MainWindow::getInstance()->createNewCanvas(); },
                                       []() { MainWindow::getInstance()->openFile(); },
                                       [](QString path) { MainWindow::getInstance()->openFile(path); },
                                       mWindow);
    mWelcomeDialog->resize(getCanvasWidget()->size());
    mWindow->takeCentralWidget();
    mWindow->setCentralWidget(mWelcomeDialog);
}

void CanvasWindow::closeWelcomeDialog() {
    if(!mWelcomeDialog) return;

    const auto mWindow = MainWindow::getInstance();
    getCanvasWidget()->resize(mWelcomeDialog->size());
    mWelcomeDialog = nullptr;
    mWindow->setCentralWidget(getCanvasWidget());
}

void CanvasWindow::changeCurrentFrameAction(const int& frame) {
    emit changeCurrentFrame(frame);
    updateAfterFrameChanged(frame);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::setClipToCanvas(const bool &bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setClipToCanvas(bT);
    mCurrentCanvas->updateAllBoxes(Animator::USER_CHANGE);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::setRasterEffectsVisible(const bool &bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setRasterEffectsVisible(bT);
    mCurrentCanvas->updateAllBoxes(Animator::USER_CHANGE);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::setPathEffectsVisible(const bool &bT) {
    if(hasNoCanvas()) return;
    mCurrentCanvas->setPathEffectsVisible(bT);
    mCurrentCanvas->updateAllBoxes(Animator::USER_CHANGE);
    queScheduledTasksAndUpdate();
}

void CanvasWindow::setResolutionFraction(const qreal &percent) {
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
    return mCurrentCanvas->getCurrentBoxesGroup();
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

        mCurrentCanvas->fitCanvasToSize();
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
            getFirstEmptyOrCachedFrameAfterFrame(mCurrentRenderFrame);
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

void CanvasWindow::setRenderingPreview(const bool &bT) {
    mRenderingPreview = bT;
    mCurrentCanvas->setRenderingPreview(bT);
}

void CanvasWindow::setPreviewing(const bool &bT) {
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
    requestUpdate();
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
    requestUpdate();
}

void CanvasWindow::nextSaveOutputFrame() {
    const auto& sCacheHandler = mCurrentCanvas->getSoundComposition()->getCacheHandler();
    const qreal fps = mCurrentCanvas->getFps();
    const int maxSec = qCeil(mMaxRenderFrame/fps);
    while(mCurrentEncodeSoundSecond <= maxSec) {
        const auto cont = sCacheHandler.atRelFrame(mCurrentEncodeSoundSecond);
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
        const auto cont = cacheHandler.atRelFrame(mCurrentEncodeFrame);
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
    for(const auto& canvas : mCanvasList) {
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

void CanvasWindow::volumeChanged(const int& value) {
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

bool CanvasWindow::dropEvent(QDropEvent *event) {
    const QMimeData* mimeData = event->mimeData();

    if(mimeData->hasUrls()) {
        event->acceptProposedAction();
        const QList<QUrl> urlList = mimeData->urls();
        for(int i = 0; i < urlList.size() && i < 32; i++) {
            try {
                const QPointF absPos = mCurrentCanvas->mapCanvasAbsToRel(
                            event->posF());
                importFile(urlList.at(i).toLocalFile(), absPos);
            } catch(const std::exception& e) {
                gPrintExceptionCritical(e);
            }
        }
        return true;
    }
    return false;
}

bool CanvasWindow::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return true;
    }
    return false;
}

bool CanvasWindow::dragMoveEvent(QDragMoveEvent *event) {
    if(event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return true;
    }
    return false;
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
            mCurrentCanvas->getCurrentBoxesGroup()->addContainedBox(importedBox);
            importedBox->moveByAbs(relDropPos);
        }
        updateHoveredElements();
    }
    queScheduledTasksAndUpdate();
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
