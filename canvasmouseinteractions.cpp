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
        BoundingBox *boxUnderMouse = NULL;
        foreach(BoundingBox *box, mChildren) {
            if(box->pointInsidePath(mPressPos)) {
                if(!(isShiftPressed()) && !box->isSelected()) {
                    clearBoxesSelection();
                }
                boxUnderMouse = box;
                break;
            }
        }
        if(boxUnderMouse == NULL) {
            if(!(isShiftPressed()) ) {
                clearBoxesSelection();
            }
            mSelecting = true;
            startSelectionAtPoint(mPressPos);
        }
        mLastPressedBox = boxUnderMouse;
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
            MovablePoint *pointUnderMouse = NULL;
            foreach (BoundingBox *box, mSelectedBoxes) {
                pointUnderMouse = box->getPointAt(mPressPos, mCurrentMode);
                if(pointUnderMouse != NULL) {
                    break;
                }
            }
            mLastPressedPoint = pointUnderMouse;


            if(mCurrentMode == CanvasMode::ADD_POINT) {
                if(mCurrentEndPoint != NULL) {
                    if(mCurrentEndPoint->isHidden()) {
                        setCurrentEndPoint(NULL);
                    }
                }
                PathPoint *pathPointUnderMouse = (PathPoint*) pointUnderMouse;
                if(pathPointUnderMouse == mCurrentEndPoint && pathPointUnderMouse != NULL) {
                    return;
                }
                if(mCurrentEndPoint == NULL && pathPointUnderMouse == NULL) {
                    VectorPath *newPath = new VectorPath(this);
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
                if (pointUnderMouse == NULL) {
                    if(!(isShiftPressed()) ) {
                        clearPointsSelection();
                    }
                    mSelecting = true;
                    startSelectionAtPoint(mPressPos);
                } else {
                    if(pointUnderMouse->isSelected()) {
                        return;
                    }
                    if(!isShiftPressed()) {
                        clearPointsSelection();
                    }
                }
            }
        } // current mode allows interaction with points
    }

    callUpdateSchedulers();
}

void Canvas::handleMovePointMouseRelease(QPointF pos) {
    if(mSelecting) {
        moveSecondSelectionPoint(pos);
        foreach (BoundingBox *box, mSelectedBoxes) {
            box->selectAndAddContainedPointsToList(mSelectionRect, &mSelectedPoints);
        }
        mSelecting = false;
    } else if(mFirstMouseMove) {
        if(isShiftPressed()) {
            if(mLastPressedPoint != NULL) {
                if(mLastPressedPoint->isSelected()) {
                    removePointFromSelection(mLastPressedPoint);
                } else {
                    addPointToSelection(mLastPressedPoint);
                }
            }
        } else {
            selectOnlyLastPressedPoint();
        }
    } else {
        startNewUndoRedoSet();
        foreach(MovablePoint *point, mSelectedPoints) {
            point->finishTransform();
        }
        finishUndoRedoSet();
    }
}


void Canvas::handleMovePathMouseRelease(QPointF pos) {
    if(mCurrentMode == CanvasMode::MOVE_PATH_ROTATE &&
            mRotPivot->isSelected()) {
        if(!mFirstMouseMove) {
            mRotPivot->finishTransform();
        }
        mRotPivot->deselect();
    } else if(mSelecting) {
        moveSecondSelectionPoint(pos);
        foreach (BoundingBox *box, mChildren) {
            if(box->isContainedIn(mSelectionRect) ) {
                addBoxToSelection(box);
            }
        }
        mSelecting = false;
    } else if(mFirstMouseMove) {
        if(isShiftPressed()) {
            if(mLastPressedBox != NULL) {
                if(mLastPressedBox->isSelected()) {
                    removeBoxFromSelection(mLastPressedBox);
                } else {
                    addBoxToSelection(mLastPressedBox);
                }
            }
        } else {
            selectOnlyLastPressedBox();
        }
    } else {
        startNewUndoRedoSet();
        foreach(BoundingBox *box, mSelectedBoxes) {
            box->finishTransform();
        }
        finishUndoRedoSet();
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
        addPointToSelection(mLastPressedPoint);
        mLastPressedPoint = NULL;
    }
    if(mFirstMouseMove) {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->startTransform();
            point->moveBy(eventPos - mPressPos);
        }
    } else {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->moveBy(eventPos - mPressPos);
        }
    }
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
            addBoxToSelection(mLastPressedBox);
            mLastPressedBox = NULL;
        }
        if(mFirstMouseMove) {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->startTransform();
                box->moveBy(scaleDistancePointByCurrentScale(eventPos - mPressPos));
            }
        } else {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->moveBy(scaleDistancePointByCurrentScale(eventPos - mPressPos));
            }
        }
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
