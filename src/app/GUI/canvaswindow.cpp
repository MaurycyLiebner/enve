// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "canvaswindow.h"
#include "canvas.h"
#include <QComboBox>
#include "mainwindow.h"
#include "GUI/BoxesList/boxscrollwidgetvisiblepart.h"
#include "singlewidgetabstraction.h"
#include "Tasks/taskexecutor.h"
#include "renderoutputwidget.h"
#include "Sound/soundcomposition.h"
#include "GUI/global.h"
#include "renderinstancesettings.h"
#include "GUI/newcanvasdialog.h"
#include "Sound/singlesound.h"
#include "svgimporter.h"
#include "filesourcescache.h"
#include <QFileDialog>
#include "videoencoder.h"
#include "usagewidget.h"
#include "memorychecker.h"
#include "memoryhandler.h"

CanvasWindow::CanvasWindow(Document &document,
                           QWidget * const parent) :
    GLWindow(parent), mDocument(document),
    mActions(*Actions::sInstance) {
    //setAttribute(Qt::WA_OpaquePaintEvent, true);
    connect(&mDocument, &Document::canvasModeSet,
            this, &CanvasWindow::setCanvasMode);

    setAcceptDrops(true);
    setMouseTracking(true);

    KFT_setFocus();
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
    if(KFT_hasFocus()) mDocument.setActiveScene(mCurrentCanvas);
    if(mCurrentCanvas) {
        mDocument.addVisibleScene(mCurrentCanvas);
        mCurrentCanvas->setIsCurrentCanvas(true);

        emit changeCanvasFrameRange(canvas->getFrameRange());
        queTasksAndUpdate();
        connect(mCurrentCanvas, &Canvas::requestCanvasMode,
                this, &CanvasWindow::setCanvasMode);
        connect(mCurrentCanvas, &Canvas::requestUpdate,
                this, qOverload<>(&CanvasWindow::update));
//        connect(mCurrentCanvas, &Canvas::prp_absFrameRangeChanged,
//                this, [this](const FrameRange& range) {
//            const int currFrame = mCurrentCanvas->anim_getCurrentAbsFrame();
//            if(range.inRange(currFrame)) update();
//        });
    }

//    if(!mCurrentCanvas) openWelcomeDialog();
//    else {
//        closeWelcomeDialog();
//        requestFitCanvasToSize();
//    }
    if(mCurrentCanvas) fitCanvasToSize();
    updateFix();
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
    if(mode == CanvasMode::boxTransform) {
        setCursor(QCursor(Qt::ArrowCursor) );
    } else if(mode == CanvasMode::pointTransform) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-node.xpm"), 0, 0) );
    } else if(mode == CanvasMode::pickFillStroke) {
        setCursor(QCursor(QPixmap(":/cursors/cursor_color_picker.png"), 2, 20) );
    } else if(mode == CanvasMode::circleCreate) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-ellipse.xpm"), 4, 4) );
    } else if(mode == CanvasMode::rectCreate) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-rect.xpm"), 4, 4) );
    } else if(mode == CanvasMode::textCreate) {
        setCursor(QCursor(QPixmap(":/cursors/cursor-text.xpm"), 4, 4) );
    } else if(mode == CanvasMode::paint) {
        updatePaintModeCursor();
    } else {
        setCursor(QCursor(QPixmap(":/cursors/cursor-pen.xpm"), 4, 4) );
    }
    if(!mCurrentCanvas) return;
    if(mMouseGrabber) {
        mCurrentCanvas->cancelCurrentTransform();
        releaseMouse();
    }
    update();
}

void CanvasWindow::queTasksAndUpdate() {
    updatePivotIfNeeded();
    update();
    Document::sInstance->actionFinished();
}

bool CanvasWindow::hasNoCanvas() {
    return !mCurrentCanvas;
}

void CanvasWindow::renameCurrentCanvas(const QString &newName) {
    if(!mCurrentCanvas) return;
    mCurrentCanvas->prp_setName(newName);
}

#include "glhelpers.h"

void CanvasWindow::renderSk(SkCanvas * const canvas) {
    if(mCurrentCanvas) {
        canvas->save();
        mCurrentCanvas->renderSk(canvas, rect(),
                                 mViewTransform, mMouseGrabber);
        canvas->restore();
    }

    if(KFT_hasFocus()) {
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(4);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(SkRect::MakeWH(width(), height()), paint);
    }
}

void CanvasWindow::tabletEvent(QTabletEvent *e) {
    if(!mCurrentCanvas) return;
    if(mDocument.fCanvasMode != CanvasMode::paint) return;
    const QPoint globalPos = mapToGlobal(QPoint(0, 0));
    const qreal x = e->hiResGlobalX() - globalPos.x();
    const qreal y = e->hiResGlobalY() - globalPos.y();
    mCurrentCanvas->tabletEvent(e, mapToCanvasCoord({x, y}));
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
    queTasksAndUpdate();
    mPrevMousePos = pos;
    if(event->button() == Qt::LeftButton) {
        mPrevPressPos = pos;
        if(mDocument.fCanvasMode == CanvasMode::paint && !mValidPaintTarget)
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
    queTasksAndUpdate();
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

    if(mDocument.fCanvasMode == CanvasMode::paint) update();
    else if(isMouseGrabber()) queTasksAndUpdate();
    else update();
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
    queTasksAndUpdate();
}

void CanvasWindow::openSettingsWindowForCurrentCanvas() {
    if(!mCurrentCanvas) return;
    const auto dialog = new CanvasSettingsDialog(mCurrentCanvas, this);
    connect(dialog, &QDialog::accepted, this, [dialog, this]() {
        dialog->applySettingsToCanvas(mCurrentCanvas);
        setCurrentCanvas(mCurrentCanvas);
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
        auto currentMode = mDocument.fCanvasMode;
        if(currentMode == CanvasMode::boxTransform) {
            if(altPressed) {
               mCurrentCanvas->deselectAllBoxesAction();
           } else {
               mCurrentCanvas->selectAllBoxesAction();
           }
        } else if(currentMode == CanvasMode::pointTransform) {
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

bool CanvasWindow::KFT_keyReleaseEvent(QKeyEvent *event) {
    if(!mCurrentCanvas) return false;
    if(mCurrentCanvas->isPreviewingOrRendering()) return false;
    if(!isMouseGrabber()) return false;
    const QPoint globalPos = QCursor::pos();
    const auto pos = mapToCanvasCoord(mapFromGlobal(globalPos));
    const KeyEvent e(pos, mPrevMousePos, mPrevPressPos, mMouseGrabber,
                     mViewTransform.m11(), globalPos,
                     QApplication::mouseButtons(), event,
                     [this]() { releaseMouse(); },
                     [this]() { grabMouse(); },
                     this);
    mCurrentCanvas->handleModifierChange(e);
    return true;
}

bool CanvasWindow::KFT_keyPressEvent(QKeyEvent *event) {
    if(!mCurrentCanvas) return false;
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
        if(mCurrentCanvas->handleModifierChange(e)) return false;
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

void CanvasWindow::setResolutionFraction(const qreal percent) {
    if(!mCurrentCanvas) return;
    mCurrentCanvas->setResolutionFraction(percent);
    mCurrentCanvas->prp_afterWholeInfluenceRangeChanged();
    mCurrentCanvas->updateAllBoxes(UpdateReason::userChange);
    queTasksAndUpdate();
}

void CanvasWindow::updatePivotIfNeeded() {
    if(!mCurrentCanvas) return;
    mCurrentCanvas->updatePivotIfNeeded();
}

void CanvasWindow::schedulePivotUpdate() {
    if(!mCurrentCanvas) return;
    mCurrentCanvas->schedulePivotUpdate();
}

ContainerBox *CanvasWindow::getCurrentGroup() {
    if(!mCurrentCanvas) return nullptr;
    return mCurrentCanvas->getCurrentGroup();
}

int CanvasWindow::getCurrentFrame() {
    if(!mCurrentCanvas) return 0;
    return mCurrentCanvas->getCurrentFrame();
}

int CanvasWindow::getMaxFrame() {
    if(!mCurrentCanvas) return 0;
    return mCurrentCanvas->getMaxFrame();
}

void CanvasWindow::dropEvent(QDropEvent *event) {
    const QMimeData* mimeData = event->mimeData();

    if(mimeData->hasUrls()) {
        event->acceptProposedAction();
        const QList<QUrl> urlList = mimeData->urls();
        for(int i = 0; i < urlList.size() && i < 32; i++) {
            try {
                const QPointF pos = mapToCanvasCoord(event->posF());
                mActions.importFile(urlList.at(i).toLocalFile(), pos);
            } catch(const std::exception& e) {
                gPrintExceptionCritical(e);
            }
        }
    }
}

void CanvasWindow::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasUrls()) {
       event->acceptProposedAction();
        KFT_setFocus();
    }
}

void CanvasWindow::dragMoveEvent(QDragMoveEvent *event) {
    if(event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CanvasWindow::grabMouse() {
    mMouseGrabber = true;
#ifndef QT_DEBUG
    QWidget::grabMouse();
#endif
    Actions::sInstance->startSmoothChange();
}

void CanvasWindow::releaseMouse() {
    mMouseGrabber = false;
#ifndef QT_DEBUG
    QWidget::releaseMouse();
#endif
    Actions::sInstance->finishSmoothChange();
}

bool CanvasWindow::isMouseGrabber() {
    return mMouseGrabber;
}
