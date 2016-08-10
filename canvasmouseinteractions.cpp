#include "canvas.h"
#include <QMouseEvent>

void Canvas::mousePressEvent(QMouseEvent *event)
{
    mFirstMouseMove = true;
    mPressPos = event->pos();
    if(isMovingPath()) {
        if(mCurrentMode == CanvasMode::MOVE_PATH_ROTATE) {
            foreach(VectorPath *path, mPaths) {
                mLastPressedPoint = (MovablePoint*) path->getPivotAt(event->pos());
                if(mLastPressedPoint != NULL) {
                    mLastPressedPoint->select();
                    callUpdateSchedulers();
                    return;
                }
            }
        }

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

    callUpdateSchedulers();
}

void Canvas::handleMovePointMouseRelease(QMouseEvent *event) {
    if(mSelecting) {
        moveSecondSelectionPoint(event->pos());
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


void Canvas::handleMovePathMouseRelease(QMouseEvent *event) {
    if(mCurrentMode == CanvasMode::MOVE_PATH_ROTATE &&
            mLastPressedPoint != NULL) {
        mLastPressedPoint->finishTransform();
        mLastPressedPoint->deselect();
        mLastPressedPoint = NULL;
    } else if(mSelecting) {
        moveSecondSelectionPoint(event->pos());
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

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if(mCurrentMode == CanvasMode::MOVE_POINT) {
        handleMovePointMouseRelease(event);
    } else if(isMovingPath()) {
        handleMovePathMouseRelease(event);
    } else if(mCurrentMode == CanvasMode::ADD_POINT) {
        if(mCurrentEndPoint != NULL) {
            if(!mCurrentEndPoint->isEndPoint()) {
                setCurrentEndPoint(NULL);
            }
        }
    }
    mLastPressedPath = NULL;
    mLastPressedPoint = NULL;

    callUpdateSchedulers();
}


void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if(mSelecting) {
        moveSecondSelectionPoint(event->pos());
    } else if(mCurrentMode == CanvasMode::MOVE_POINT) {
        if(mLastPressedPoint != NULL) {
            addPointToSelection(mLastPressedPoint);
            mLastPressedPoint = NULL;
        }
        if(mFirstMouseMove) {
            foreach(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->moveBy(event->pos() - mPressPos);
            }
        } else {
            foreach(MovablePoint *point, mSelectedPoints) {
                point->moveBy(event->pos() - mPressPos);
            }
        }
    } else if(isMovingPath()) {
        if(mCurrentMode == CanvasMode::MOVE_PATH_ROTATE &&
                mLastPressedPoint != NULL) {
            mLastPressedPoint->moveBy(event->pos() - mPressPos);
        } else {
            if(mLastPressedPath != NULL) {
                addPathToSelection(mLastPressedPath);
                mLastPressedPath = NULL;
            }
            if(mFirstMouseMove) {
                foreach(VectorPath *path, mSelectedPaths) {
                    path->startTransform();
                    path->moveBy(event->pos() - mPressPos);
                    path->updateMappedPathIfNeeded();
                }
            } else {
                foreach(VectorPath *path, mSelectedPaths) {
                    path->moveBy(event->pos() - mPressPos);
                    path->updateMappedPathIfNeeded();
                }
            }
        }
    } else if(mCurrentMode == CanvasMode::ADD_POINT) {
        mCurrentEndPoint->moveEndCtrlPtToAbsPos(event->pos());
    }
    mPressPos = event->pos();
    mFirstMouseMove = false;

    callUpdateSchedulers();
}
