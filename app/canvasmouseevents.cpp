#include "canvas.h"
#include "GUI/mainwindow.h"

#include "Boxes/paintbox.h"
#include "Boxes/textbox.h"
#include "Boxes/rectangle.h"
#include "Boxes/circle.h"
#include "Boxes/bone.h"

#include "MovablePoints/pathpivot.h"
#include "MovablePoints/nodepoint.h"

#include "edge.h"

void Canvas::mousePressEvent(const QMouseEvent * const event) {
    if(isPreviewingOrRendering()) return;
    setLastMouseEventPosAbs(event->pos());
    setLastMousePressPosAbs(event->pos());
    setCurrentMouseEventPosAbs(event->pos());
    if(mCurrentMode == PAINT_MODE) {
        if(mStylusDrawing) return;
        if(event->button() == Qt::LeftButton) {
            for(const auto& box : mSelectedBoxes) {
                if(box->SWT_isPaintBox()) {
                    const auto paintBox = GetAsPtr(box, PaintBox);
                    paintBox->mousePressEvent(mLastMouseEventPosRel.x(),
                                         mLastMouseEventPosRel.y(),
                                         event->timestamp(), 0.5,
                                         mCurrentBrush);
                }
            }
        }
    } else {
        if(event->button() == Qt::LeftButton) {
            handleLeftButtonMousePress();
        } else if(event->button() == Qt::RightButton) {
            handleRightButtonMousePress(event);
        }
    }

    callUpdateSchedulers();
}

void Canvas::handlePaintLeftButtonMoveEvent(const QMouseEvent * const event) {
    for(const auto& box : mSelectedBoxes) {
        if(box->SWT_isPaintBox()) {
            const auto paintBox = GetAsPtr(box, PaintBox);
            paintBox->mouseMoveEvent(mCurrentMouseEventPosRel.x(),
                                     mCurrentMouseEventPosRel.y(),
                                     event->timestamp(),
                                     false, mCurrentBrush);
        }
    }
    callUpdateSchedulers();
}

void Canvas::mouseMoveEvent(const QMouseEvent * const event) {
    if(isPreviewingOrRendering()) return;
    setCurrentMouseEventPosAbs(event->pos());

    if(!(event->buttons() & Qt::MiddleButton) &&
       !(event->buttons() & Qt::RightButton) &&
       !(event->buttons() & Qt::LeftButton) &&
       !mIsMouseGrabbing) {
        const auto lastHoveredBox = mHoveredBox;
        const auto lastHoveredPoint = mHoveredPoint_d;
        const auto lastNSegment = mHoveredNormalSegment;

        updateHoveredElements();

        setLastMouseEventPosAbs(event->pos());
        if(mHoveredPoint_d != lastHoveredPoint ||
           mHoveredBox != lastHoveredBox ||
           mHoveredNormalSegment != lastNSegment) {
            callUpdateSchedulers();
        }
        return;
    }

    if(event->buttons() & Qt::MiddleButton) {
        moveByRel(mCurrentMouseEventPosRel - mLastMouseEventPosRel);
    } else if(mCurrentMode == PAINT_MODE && event->buttons() & Qt::LeftButton)  {
        handlePaintLeftButtonMoveEvent(event);
    } else if(event->buttons() & Qt::LeftButton || mIsMouseGrabbing) {
        if(mMovesToSkip > 0) {
            mMovesToSkip--;
            return;
        }
        if(mFirstMouseMove && event->buttons() & Qt::LeftButton) {
            if((mCurrentMode == CanvasMode::MOVE_POINT &&
                !mHoveredPoint_d && !mHoveredNormalSegment.isValid()) ||
               (mCurrentMode == CanvasMode::MOVE_PATH &&
                !mHoveredBox && !mHoveredPoint_d)) {
                startSelectionAtPoint(mLastMouseEventPosRel);
            }
        }
        if(mSelecting) {
            moveSecondSelectionPoint(mCurrentMouseEventPosRel);
        } else if(mCurrentMode == CanvasMode::MOVE_POINT ||
                  mCurrentMode == CanvasMode::ADD_PARTICLE_BOX ||
                  mCurrentMode == CanvasMode::ADD_PAINT_BOX ||
                  mCurrentMode == CanvasMode::ADD_BONE) {
            handleMovePointMouseMove();
        } else if(isMovingPath()) {
            if(!mLastPressedPoint) {
                handleMovePathMouseMove();
            } else {
                handleMovePointMouseMove();
            }
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMouseMove();
        } else if(mCurrentMode == CanvasMode::ADD_SMART_POINT) {
            handleAddSmartPointMouseMove();
        } else if(mCurrentMode == CanvasMode::ADD_CIRCLE) {
            if(isShiftPressed()) {
                const qreal lenR = pointToLen(mCurrentMouseEventPosRel -
                                              mLastPressPosRel);
                mCurrentCircle->moveRadiusesByAbs(QPointF(lenR, lenR));
            } else {
                mCurrentCircle->moveRadiusesByAbs(mCurrentMouseEventPosRel -
                                                  mLastPressPosRel);
            }
        } else if(mCurrentMode == CanvasMode::ADD_RECTANGLE) {
            if(isShiftPressed()) {
                QPointF trans = mCurrentMouseEventPosRel - mLastPressPosRel;
                const qreal valF = qMax(trans.x(), trans.y());
                trans = QPointF(valF, valF);
                mCurrentRectangle->moveSizePointByAbs(trans);
            } else {
                mCurrentRectangle->moveSizePointByAbs(mCurrentMouseEventPosRel -
                                                      mLastPressPosRel);
            }
        }
    }
    mFirstMouseMove = false;
    setLastMouseEventPosAbs(event->pos());
    callUpdateSchedulers();
}

void Canvas::handlePaintModeMouseRelease() {
    for(const auto& box : mSelectedBoxes) {
        if(box->SWT_isPaintBox()) {
            const auto paintBox = GetAsPtr(box, PaintBox);
            paintBox->mouseReleaseEvent();
        }
    }
    callUpdateSchedulers();
}

void Canvas::mouseReleaseEvent(const QMouseEvent * const event) {
    if(isPreviewingOrRendering()) return;
    if(event->button() != Qt::LeftButton) return;
    schedulePivotUpdate();
    if(mCurrentMode == PAINT_MODE) {
        handlePaintModeMouseRelease();
        return;
    }
    setCurrentMouseEventPosAbs(event->pos());
    mXOnlyTransform = false;
    mYOnlyTransform = false;
    if(mValueInput.inputEnabled()) mFirstMouseMove = false;
    mValueInput.clearAndDisableInput();

    handleMouseRelease();

    mLastPressedBone = nullptr;
    mLastPressedBox = nullptr;
    mHoveredPoint_d = mLastPressedPoint;
    mLastPressedPoint = nullptr;

    setLastMouseEventPosAbs(event->pos());
    callUpdateSchedulers();
}

void Canvas::wheelEvent(const QWheelEvent * const event) {
    if(isPreviewingOrRendering()) return;
    if(event->delta() > 0) {
        zoomCanvas(1.1, event->posF());
    } else {
        zoomCanvas(0.9, event->posF());
    }

    callUpdateSchedulers();
}

void Canvas::mouseDoubleClickEvent(const QMouseEvent * const e) {
    if(e->modifiers() & Qt::ShiftModifier) return;
    mDoubleClick = true;

    BoundingBox * const boxAt =
            mCurrentBoxesGroup->getBoxAt(mLastPressPosRel);
    if(!boxAt) {
        if(!mHoveredPoint_d && !mHoveredNormalSegment.isValid()) {
            if(mCurrentBoxesGroup != this) {
                setCurrentBoxesGroup(mCurrentBoxesGroup->getParentGroup());
            }
        }
    } else {
        if(boxAt->SWT_isBoxesGroup()) {
            setCurrentBoxesGroup(static_cast<BoxesGroup*>(boxAt));
            updateHoveredElements();
        } else if((mCurrentMode == MOVE_PATH ||
                   mCurrentMode == MOVE_POINT) &&
                  boxAt->SWT_isTextBox()) {
            releaseMouseAndDontTrack();
            GetAsPtr(boxAt, TextBox)->openTextEditor(mMainWindow);
        } else if(mCurrentMode == MOVE_PATH &&
                  boxAt->SWT_isVectorPath()) {
            mCanvasWindow->setCanvasMode(MOVE_POINT);
        }
    }

    callUpdateSchedulers();
}

void Canvas::tabletEvent(const QTabletEvent * const e,
                         const QPointF &absPos) {
    if(mCurrentMode != PAINT_MODE || e->buttons() & Qt::MiddleButton) return;
    setCurrentMouseEventPosAbs(absPos);
    if(e->type() == QEvent::TabletPress) {
        if(e->button() == Qt::RightButton) return;
        if(e->button() == Qt::LeftButton) {
            mStylusDrawing = true;

            for(const auto& box : mSelectedBoxes) {
                if(box->SWT_isPaintBox()) {
                    const auto paintBox = GetAsPtr(box, PaintBox);
                    paintBox->mousePressEvent(mLastMouseEventPosRel.x(),
                                         mLastMouseEventPosRel.y(),
                                         e->timestamp(),
                                         e->pressure(),
                                         mCurrentBrush);
                }
            }
        }
    } else if(e->type() == QEvent::TabletRelease) {
        if(e->button() == Qt::LeftButton) {
            mStylusDrawing = false;
            for(const auto& box : mSelectedBoxes) {
                if(box->SWT_isPaintBox()) {
                    const auto paintBox = GetAsPtr(box, PaintBox);
                    paintBox->mouseReleaseEvent();
                }
            }
        }
    } else if(mStylusDrawing) {
        for(const auto& box : mSelectedBoxes) {
            if(box->SWT_isPaintBox()) {
                const auto paintBox = GetAsPtr(box, PaintBox);
                paintBox->tabletMoveEvent(mLastMouseEventPosRel.x(),
                                      mLastMouseEventPosRel.y(),
                                      e->timestamp(),
                                      e->pressure(),
                                      e->pointerType() == QTabletEvent::Eraser,
                                      mCurrentBrush);
            }
        }
    } // else if
    setLastMouseEventPosAbs(absPos);
    callUpdateSchedulers();
}
