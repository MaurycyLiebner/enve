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
        VectorPath *pathUnderMouse = NULL;
        foreach(VectorPath *path, mPaths) {
            if(path->pointInsidePath(mPressPos)) {
                if(!(isShiftPressed()) && !path->isSelected()) {
                    clearPathsSelection();
                }
                pathUnderMouse = path;
                break;
            }
        }
        if(pathUnderMouse == NULL) {
            if(!(isShiftPressed()) ) {
                clearPathsSelection();
            }
            mSelecting = true;
            startSelectionAtPoint(mPressPos);
        }
        mLastPressedPath = pathUnderMouse;
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
            foreach (VectorPath *path, mSelectedPaths) {
                pointUnderMouse = path->getPointAt(mPressPos, mCurrentMode);
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
                    addPath(newPath);
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
        foreach (VectorPath *path, mSelectedPaths) {
            path->selectAndAddContainedPointsToList(mSelectionRect, &mSelectedPoints);
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
        foreach (VectorPath *path, mPaths) {
            if(path->isContainedIn(mSelectionRect) ) {
                addPathToSelection(path);
            }
        }
        mSelecting = false;
    } else if(mFirstMouseMove) {
        if(isShiftPressed()) {
            if(mLastPressedPath != NULL) {
                if(mLastPressedPath->isSelected()) {
                    removePathFromSelection(mLastPressedPath);
                } else {
                    addPathToSelection(mLastPressedPath);
                }
            }
        } else {
            selectOnlyLastPressedPath();
        }
    } else {
        startNewUndoRedoSet();
        foreach(VectorPath *path, mSelectedPaths) {
            path->finishTransform();
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
    mLastPressedPath = NULL;
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
        if(mLastPressedPath != NULL) {
            addPathToSelection(mLastPressedPath);
            mLastPressedPath = NULL;
        }
        if(mFirstMouseMove) {
            foreach(VectorPath *path, mSelectedPaths) {
                path->startTransform();
                path->moveBy(scaleDistancePointByCurrentScale(eventPos - mPressPos));
                path->updateMappedPathIfNeeded();
            }
        } else {
            foreach(VectorPath *path, mSelectedPaths) {
                path->moveBy(scaleDistancePointByCurrentScale(eventPos - mPressPos));
                path->updateMappedPathIfNeeded();
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
