#include "canvas.h"
#include <QMouseEvent>
#include "pathpivot.h"

QPointF Canvas::scaleDistancePointByCurrentScale(QPointF point) {
    return point/mTransformMatrix.m11();
}

void Canvas::handleMovePathMousePressEvent() {
    if(mCurrentMode == CanvasMode::MOVE_PATH_ROTATE) {
        if(mRotPivot->isPointAt(mPressPos)) {
            mRotPivot->select();
        }
    } else {
        mLastPressedBox = mCurrentBoxesGroup->getBoxAt(mPressPos);
        if(mLastPressedBox != NULL) {
            if(!(isShiftPressed()) && !mLastPressedBox->isSelected()) {
                mCurrentBoxesGroup->clearBoxesSelection();
            }
        }
        if(mLastPressedBox == NULL) {
            if(!(isShiftPressed()) ) {
                mCurrentBoxesGroup->clearBoxesSelection();
            }
            mSelecting = true;
            startSelectionAtPoint(mPressPos);
        }
    }
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    mFirstMouseMove = true;
    mPressPos = event->pos();
    if(event->button() == Qt::MiddleButton) {
        return;
    } else {
        if(isMovingPath()) {
            handleMovePathMousePressEvent();
        } else {
            mLastPressedPoint = mCurrentBoxesGroup->getPointAt(mPressPos, mCurrentMode);


            if(mCurrentMode == CanvasMode::ADD_POINT) {
                if(mCurrentEndPoint != NULL) {
                    if(mCurrentEndPoint->isHidden()) {
                        setCurrentEndPoint(NULL);
                    }
                }
                PathPoint *pathPointUnderMouse = (PathPoint*) mLastPressedPoint;
                if(pathPointUnderMouse == mCurrentEndPoint && pathPointUnderMouse != NULL) {
                    return;
                }
                if(mCurrentEndPoint == NULL && pathPointUnderMouse == NULL) {
                    VectorPath *newPath = new VectorPath(mCurrentBoxesGroup);
                    mCurrentBoxesGroup->addBoxToSelection(newPath);
                    setCurrentEndPoint(newPath->addPoint(mPressPos, mCurrentEndPoint) );
                } else {
                    if(pathPointUnderMouse == NULL) {
                        setCurrentEndPoint(mCurrentEndPoint->addPoint(mPressPos) );
                    } else if(mCurrentEndPoint == NULL) {
                            setCurrentEndPoint(pathPointUnderMouse);
                    } else {
                        if(mCurrentEndPoint->getParentPath() == pathPointUnderMouse->getParentPath())
                        {
                            pathPointUnderMouse->connectToPoint(mCurrentEndPoint);
                        }
                        else {
                            connectPointsFromDifferentPaths(mCurrentEndPoint, pathPointUnderMouse);
                        }
                        setCurrentEndPoint(pathPointUnderMouse);
                    }
                } // pats is not null
            } // point adding mode
            else if (mCurrentMode == CanvasMode::MOVE_POINT) {
                if (mLastPressedPoint == NULL) {
                    if(!(isShiftPressed()) ) {
                        mCurrentBoxesGroup->clearPointsSelection();
                    }
                    mSelecting = true;
                    startSelectionAtPoint(mPressPos);
                } else {
                    if(mLastPressedPoint->isSelected()) {
                        return;
                    }
                    if(!isShiftPressed()) {
                        mCurrentBoxesGroup->clearPointsSelection();
                    }
                }
            }
        } // current mode allows interaction with points
    }

    callUpdateSchedulers();
}

void Canvas::handleMovePointMouseRelease(QPointF pos) {
    if(mSelecting) {
        mSelecting = false;
        if(mFirstMouseMove) {
            mLastPressedBox = mCurrentBoxesGroup->getBoxAt(pos);
            if((mLastPressedBox == NULL) ? true : mLastPressedBox->isGroup()) {
                mLastPressedBox = getBoxAtFromAllAncestors(pos);
                if(mLastPressedBox != NULL) {
                    setCurrentBoxesGroup((BoxesGroup*) mLastPressedBox->getParent());
                }
            }
            if(mLastPressedBox != NULL) {
                if(isShiftPressed()) {
                    if(mLastPressedBox->isSelected()) {
                        mCurrentBoxesGroup->removeBoxFromSelection(mLastPressedBox);
                    } else {
                        mCurrentBoxesGroup->addBoxToSelection(mLastPressedBox);
                    }
                } else {
                    selectOnlyLastPressedBox();
                }
            }
            return;
        }
        moveSecondSelectionPoint(pos);
        mCurrentBoxesGroup->selectAndAddContainedPointsToSelection(mSelectionRect);
    } else if(mFirstMouseMove) {
        if(isShiftPressed()) {
            if(mLastPressedPoint != NULL) {
                if(mLastPressedPoint->isSelected()) {
                    mCurrentBoxesGroup->removePointFromSelection(mLastPressedPoint);
                } else {
                    mCurrentBoxesGroup->addPointToSelection(mLastPressedPoint);
                }
            }
        } else {
            selectOnlyLastPressedPoint();
        }
    } else {
        mCurrentBoxesGroup->finishSelectedPointsTransform();
    }
}


void Canvas::handleMovePathMouseRelease(QPointF pos) {
    if(mCurrentMode == CanvasMode::MOVE_PATH_ROTATE &&
            mRotPivot->isSelected()) {
        if(!mFirstMouseMove) {
            mRotPivot->finishTransform();
        }
        mRotPivot->deselect();
    } else if(mFirstMouseMove) {
        mSelecting = false;
        if(isShiftPressed() && mLastPressedBox != NULL) {
            if(mLastPressedBox->isSelected()) {
                mCurrentBoxesGroup->removeBoxFromSelection(mLastPressedBox);
            } else {
                mCurrentBoxesGroup->addBoxToSelection(mLastPressedBox);
            }
        } else {
            selectOnlyLastPressedBox();
        }
    } else if(mSelecting) {
        moveSecondSelectionPoint(pos);
        mCurrentBoxesGroup->addContainedBoxesToSelection(mSelectionRect);
        mSelecting = false;
    } else {
        mCurrentBoxesGroup->finishSelectedBoxesTransform();
    }
}

void Canvas::handleAddPointMouseRelease() {
    if(mCurrentEndPoint != NULL) {
        if(!mCurrentEndPoint->isEndPoint()) {
            setCurrentEndPoint(NULL);
        }
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF eventPos = event->pos();
    if(event->button() == Qt::MiddleButton) {

    } else {
        if(mCurrentMode == CanvasMode::MOVE_POINT) {
            handleMovePointMouseRelease(eventPos);
        } else if(isMovingPath()) {
            handleMovePathMouseRelease(eventPos);
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMouseRelease();
        }
    }
    mLastPressedBox = NULL;
    mLastPressedPoint = NULL;

    callUpdateSchedulers();
}

void Canvas::handleMovePointMouseMove(QPointF eventPos) {
    if(mLastPressedPoint != NULL) {
        mCurrentBoxesGroup->addPointToSelection(mLastPressedPoint);
        mLastPressedPoint = NULL;
    }
    mCurrentBoxesGroup->moveSelectedPointsBy(eventPos - mPressPos, mFirstMouseMove);
}

void Canvas::handleMovePathMouseMove(QPointF eventPos) {
    if(mCurrentMode == CanvasMode::MOVE_PATH_ROTATE &&
            mRotPivot->isSelected()) {
        if(mFirstMouseMove) {
            mRotPivot->startTransform();
        }
        mRotPivot->moveBy(scaleDistancePointByCurrentScale(eventPos - mPressPos));
    } else {
        if(mLastPressedBox != NULL) {
            mCurrentBoxesGroup->addBoxToSelection(mLastPressedBox);
            mLastPressedBox = NULL;
        }
        mCurrentBoxesGroup->moveSelectedBoxesBy(
                    scaleDistancePointByCurrentScale(eventPos - mPressPos),
                    mFirstMouseMove);
    }
}

void Canvas::handleAddPointMouseMove(QPointF eventPos) {
    mCurrentEndPoint->moveEndCtrlPtToAbsPos(eventPos);
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    QPointF eventPos = event->pos();
    if(event->buttons() & Qt::MiddleButton) {
        moveBy(scaleDistancePointByCurrentScale(event->pos() - mPressPos));
        scheduleRepaint();
    } else {
        if(mSelecting) {
            moveSecondSelectionPoint(eventPos);
        } else if(mCurrentMode == CanvasMode::MOVE_POINT) {
            handleMovePointMouseMove(eventPos);
        } else if(isMovingPath()) {
            handleMovePathMouseMove(eventPos);
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMouseMove(eventPos);
        }
    }
    mPressPos = event->pos();
    mFirstMouseMove = false;

    callUpdateSchedulers();
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    QPointF zoomOrigin = (getAbsolutePos() - event->posF())/mTransformMatrix.m11();
    if(event->delta() > 0) {
        scale(1.2, zoomOrigin);
    } else {
        scale(0.8, zoomOrigin);
    }
    mVisibleHeight = mTransformMatrix.m22()*mHeight;
    mVisibleWidth = mTransformMatrix.m11()*mWidth;
    scheduleRepaint();
    callUpdateSchedulers();
}

void Canvas::mouseDoubleClickEvent(QMouseEvent *event)
{
    BoundingBox *boxAt = mCurrentBoxesGroup->getBoxAt(event->pos());
    if(boxAt == NULL) {
        if(mCurrentBoxesGroup != this) {
            setCurrentBoxesGroup((BoxesGroup*) mCurrentBoxesGroup->getParent());
        }
    } else {
        if(boxAt->isGroup()) {
            setCurrentBoxesGroup((BoxesGroup*) boxAt);
        } else if(mCurrentMode == MOVE_PATH_ROTATE) {
            setCanvasMode(MOVE_PATH_SCALE);
        } else if(mCurrentMode == MOVE_PATH_SCALE) {
            setCanvasMode(MOVE_PATH_ROTATE);
        }
    }
    scheduleRepaint();
    callUpdateSchedulers();
}
