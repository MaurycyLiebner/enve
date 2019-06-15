#include "canvas.h"
#include "GUI/mainwindow.h"

#include "Boxes/paintbox.h"
#include "Boxes/textbox.h"
#include "Boxes/rectangle.h"
#include "Boxes/circle.h"

#include "MovablePoints/pathpivot.h"

void Canvas::mousePressEvent(const QMouseEvent * const event) {
    if(isPreviewingOrRendering()) return;
    const auto button = event->button();
    if(mIsMouseGrabbing && button == Qt::LeftButton) return;
    setLastMouseEventPosAbs(event->pos());
    setLastMousePressPosAbs(event->pos());
    setCurrentMouseEventPosAbs(event->pos());
    if(mCurrentMode == PAINT_MODE) {
        if(mStylusDrawing) return;
        if(button == Qt::LeftButton) {
            paintPress(event->timestamp(), 0.5, 0, 0);
        }
    } else {
        if(button == Qt::LeftButton) {
            handleLeftButtonMousePress();
        } else if(button == Qt::RightButton) {
            handleRightButtonMousePress(event->globalPos());
        }
    }
}

void Canvas::mouseMoveEvent(const QMouseEvent * const event) {
    if(isPreviewingOrRendering()) return;
    setCurrentMouseEventPosAbs(event->pos());

    const bool leftPressed = event->buttons() & Qt::LeftButton;
    const bool middlePressed = event->buttons() & Qt::MiddleButton;

    if(!middlePressed && !leftPressed && !mIsMouseGrabbing) {
        const auto lastHoveredBox = mHoveredBox;
        const auto lastHoveredPoint = mHoveredPoint_d;
        const auto lastNSegment = mHoveredNormalSegment;

        updateHoveredElements();

        setLastMouseEventPosAbs(event->pos());
        return;
    }

    if(middlePressed) {
        moveByRel(mCurrentMouseEventPosRel - mLastMouseEventPosRel);
    } else if(mCurrentMode == PAINT_MODE && leftPressed)  {
        paintMove(event->timestamp(), 1, 0, 0);
        return mActiveWindow->requestUpdate();
    } else if(leftPressed || mIsMouseGrabbing) {
        if(mMovesToSkip > 0) {
            mMovesToSkip--;
            return;
        }
        if(mFirstMouseMove && leftPressed) {
            if((mCurrentMode == CanvasMode::MOVE_POINT &&
                !mHoveredPoint_d && !mHoveredNormalSegment.isValid()) ||
               (mCurrentMode == CanvasMode::MOVE_BOX &&
                !mHoveredBox && !mHoveredPoint_d)) {
                startSelectionAtPoint(mLastMouseEventPosRel);
            }
        }
        if(mSelecting) {
            moveSecondSelectionPoint(mCurrentMouseEventPosRel);
        } else if(mCurrentMode == CanvasMode::MOVE_POINT ||
                  mCurrentMode == CanvasMode::ADD_PARTICLE_BOX ||
                  mCurrentMode == CanvasMode::ADD_PAINT_BOX) {
            handleMovePointMouseMove();
        } else if(mCurrentMode == CanvasMode::MOVE_BOX) {
            if(!mLastPressedPoint) {
                handleMovePathMouseMove();
            } else {
                handleMovePointMouseMove();
            }
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
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
    if(!mSelecting && !mIsMouseGrabbing && leftPressed) grabMouseAndTrack();
}

void Canvas::mouseReleaseEvent(const QMouseEvent * const event) {
    if(isPreviewingOrRendering()) return;
    if(event->button() != Qt::LeftButton) return;
    schedulePivotUpdate();
    if(mCurrentMode == PAINT_MODE) return;
    setCurrentMouseEventPosAbs(event->pos());
    if(mValueInput.inputEnabled()) mFirstMouseMove = false;
    mValueInput.clearAndDisableInput();

    handleMouseRelease();

    mLastPressedBox = nullptr;
    mHoveredPoint_d = mLastPressedPoint;
    mLastPressedPoint = nullptr;

    setLastMouseEventPosAbs(event->pos());
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
        if(boxAt->SWT_isContainerBox()) {
            setCurrentBoxesGroup(static_cast<ContainerBox*>(boxAt));
            updateHoveredElements();
        } else if((mCurrentMode == MOVE_BOX ||
                   mCurrentMode == MOVE_POINT) &&
                  boxAt->SWT_isTextBox()) {
            releaseMouseAndDontTrack();
            GetAsPtr(boxAt, TextBox)->openTextEditor(mMainWindow);
        } else if(mCurrentMode == MOVE_BOX &&
                  boxAt->SWT_isSmartVectorPath()) {
            mActiveWindow->setCanvasMode(MOVE_POINT);
        }
    }
}

void Canvas::tabletEvent(const QTabletEvent * const e,
                         const QPointF &absPos) {
    if(mCurrentMode != PAINT_MODE) return;
    const auto type = e->type();
    setCurrentMouseEventPosAbs(absPos);

    if(type == QEvent::TabletRelease ||
       e->buttons() & Qt::MiddleButton) {
        mStylusDrawing = false;
    } else if(e->type() == QEvent::TabletPress) {
        if(e->button() == Qt::RightButton) return;
        if(e->button() == Qt::LeftButton) {
            mStylusDrawing = true;

            paintPress(e->timestamp(), e->pressure(),
                       e->xTilt(), e->yTilt());
        }
    } else if(type == QEvent::TabletMove && mStylusDrawing) {
        paintMove(e->timestamp(), e->pressure(),
                  e->xTilt(), e->yTilt());
    }
    setLastMouseEventPosAbs(absPos);
}
