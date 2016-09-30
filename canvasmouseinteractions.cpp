#include "canvas.h"
#include <QMouseEvent>
#include "pathpivot.h"

QPointF Canvas::scaleDistancePointByCurrentScale(QPointF point) {
    return point/mCombinedTransformMatrix.m11();
}

void Canvas::handleMovePathMousePressEvent() {
    if((mCurrentMode == CanvasMode::MOVE_PATH) ?
            !mRotPivot->handleMousePress(mLastMouseEventPos) : true) {
        mLastPressedBox = mCurrentBoxesGroup->getBoxAt(mLastMouseEventPos);
        if(mLastPressedBox == NULL) {
            if(!isShiftPressed() ) {
                mCurrentBoxesGroup->clearBoxesSelection();
            }
            mSelecting = true;
            startSelectionAtPoint(mLastMouseEventPos);
        } else {
            if(!isShiftPressed() && !mLastPressedBox->isSelected()) {
                mCurrentBoxesGroup->clearBoxesSelection();
            }
        }
    }
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if(mIsMouseGrabbing) {
        mIsMouseGrabbing = false;
        setMouseTracking(false);
        if(event->button() == Qt::RightButton) {
            mCancelTransform = true;
        }
        return;
    }
    mDoubleClick = false;
    mMovesToSkip = 2;
    mFirstMouseMove = true;
    mLastMouseEventPos = event->pos();
    mLastPressPos = mLastMouseEventPos;
    if(event->button() == Qt::MiddleButton) {
        return;
    } else {
        if(isMovingPath()) {
            handleMovePathMousePressEvent();
        } else if(mCurrentMode == PICK_PATH_SETTINGS) {
            mLastPressedBox = getPathAtFromAllAncestors(mLastPressPos);
        } else {
            mLastPressedPoint = mCurrentBoxesGroup->getPointAt(mLastMouseEventPos, mCurrentMode);

            if(mCurrentMode == CanvasMode::ADD_POINT) {
                if(mCurrentEndPoint != NULL) {
                    if(mCurrentEndPoint->isHidden()) {
                        setCurrentEndPoint(NULL);
                    }
                }
                PathPoint *pathPointUnderMouse = (PathPoint*) mLastPressedPoint;
                if( (pathPointUnderMouse == NULL) ? false :
                        !pathPointUnderMouse->isEndPoint() ) {
                    pathPointUnderMouse = NULL;
                }
                if(pathPointUnderMouse == mCurrentEndPoint && pathPointUnderMouse != NULL) {
                    return;
                }
                if(mCurrentEndPoint == NULL && pathPointUnderMouse == NULL) {
                    VectorPath *newPath = new VectorPath(mCurrentBoxesGroup);
                    mCurrentBoxesGroup->clearBoxesSelection();
                    mCurrentBoxesGroup->addBoxToSelection(newPath);
                    setCurrentEndPoint(newPath->addPointAbsPos(mLastMouseEventPos, mCurrentEndPoint) );
                } else {
                    if(pathPointUnderMouse == NULL) {
                        setCurrentEndPoint(mCurrentEndPoint->addPointAbsPos(mLastMouseEventPos) );
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
                    if(isCtrlPressed() ) {
                        mCurrentBoxesGroup->clearPointsSelection();
                        mLastPressedPoint = mCurrentBoxesGroup->
                                createNewPointOnLineNearSelected(mLastPressPos);
                    } else {
                        mCurrentEdge = mCurrentBoxesGroup->getPressedEdge(
                                                                mLastPressPos);
                        if(mCurrentEdge == NULL) {
                            mSelecting = true;
                            startSelectionAtPoint(mLastMouseEventPos);
                        } else {
                            mCurrentBoxesGroup->clearPointsSelection();
                        }
                    }
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
    if(mCancelTransform) {
        mCancelTransform = false;
        mCurrentBoxesGroup->cancelSelectedPointsTransform();
    } else if(mSelecting) {
        mSelecting = false;
        if(mFirstMouseMove) {
            mLastPressedBox = mCurrentBoxesGroup->getBoxAt(pos);
            if((mLastPressedBox == NULL) ? true : mLastPressedBox->isGroup()) {
                BoundingBox *pressedBox = getPathAtFromAllAncestors(pos);
                if(pressedBox == NULL) {
                    if(!(isShiftPressed()) ) {
                        mCurrentBoxesGroup->clearPointsSelectionOrDeselect();
                    }
                } else {
                    mCurrentBoxesGroup->clearPointsSelection();
                    setCurrentBoxesGroup((BoxesGroup*) pressedBox->getParent());
                    mCurrentBoxesGroup->addBoxToSelection(pressedBox);
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
                    mCurrentBoxesGroup->clearPointsSelection();
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
    if(mCurrentMode == CanvasMode::MOVE_PATH &&
            mRotPivot->isSelected()) {
        if(!mFirstMouseMove) {
            mRotPivot->finishTransform();
        }
        mRotPivot->deselect();
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
        mRotPivot->handleMouseRelease();
        if(mCancelTransform) {
            mCancelTransform = false;
            mCurrentBoxesGroup->cancelSelectedBoxesTransform();
        } else {
            mCurrentBoxesGroup->finishSelectedBoxesTransform();
        }
    } else if(mCancelTransform) {
        mCancelTransform = false;
        mCurrentBoxesGroup->cancelSelectedBoxesTransform();
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
    if(!mDoubleClick) {
        QPointF eventPos = event->pos();
        if(event->button() == Qt::MiddleButton) {

        } else {
            if(mCurrentMode == CanvasMode::MOVE_POINT) {
                handleMovePointMouseRelease(eventPos);
            } else if(isMovingPath()) {
                handleMovePathMouseRelease(eventPos);
            } else if(mCurrentMode == CanvasMode::ADD_POINT) {
                handleAddPointMouseRelease();
            } else if(mCurrentMode == PICK_PATH_SETTINGS) {
                if(mLastPressedBox != NULL) {
                    mFillStrokeSettingsWidget->loadSettingsFromPath(
                                (VectorPath*) mLastPressedBox);
                }
                setCanvasMode(MOVE_PATH);
            }
        }
    }
    mLastPressedBox = NULL;
    mLastPressedPoint = NULL;
    if(mCurrentEdge != NULL) {
        delete mCurrentEdge;
        mCurrentEdge = NULL;
    }

    callUpdateSchedulers();
}

void Canvas::handleMovePointMouseMove(QPointF eventPos) {
    if(mCurrentEdge != NULL) {
        mCurrentEdge->makePassThrough(eventPos);
    } else {
        if(mLastPressedPoint != NULL) {
            mCurrentBoxesGroup->addPointToSelection(mLastPressedPoint);
            mLastPressedPoint = NULL;
        }
        mCurrentBoxesGroup->moveSelectedPointsBy(eventPos - mLastMouseEventPos,
                                                 mFirstMouseMove);
    }
}

void Canvas::setPivotPositionForSelected() {
    mCurrentBoxesGroup->setSelectedPivotAbsPos(mRotPivot->getAbsolutePos());
}

void Canvas::handleMovePathMouseMove(QPointF eventPos) {
    if(mCurrentMode == CanvasMode::MOVE_PATH &&
            mRotPivot->isSelected()) {
        if(mFirstMouseMove) {
            mRotPivot->startTransform();
        }
        mRotPivot->moveBy(eventPos - mLastMouseEventPos);
    } else if((mCurrentMode == CanvasMode::MOVE_PATH && mRotPivot->isRotating()) ||
              (mCurrentMode == CanvasMode::MOVE_PATH && mRotPivot->isScaling()) ) {
        mRotPivot->handleMouseMove(eventPos, mLastPressPos, mFirstMouseMove);
    } else {
        if(mLastPressedBox != NULL) {
            mCurrentBoxesGroup->addBoxToSelection(mLastPressedBox);
            mLastPressedBox = NULL;
        }
        mCurrentBoxesGroup->moveSelectedBoxesBy(eventPos - mLastMouseEventPos,
                    mFirstMouseMove);
    }
}

void Canvas::handleAddPointMouseMove(QPointF eventPos) {
    if(mCurrentEndPoint == NULL) return;
    mCurrentEndPoint->setEndCtrlPtEnabled(true);
    mCurrentEndPoint->setStartCtrlPtEnabled(true);
    mCurrentEndPoint->moveEndCtrlPtToAbsPos(eventPos);
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if(mMovesToSkip > 0) {
        mMovesToSkip--;
        return;
    }
    QPointF eventPos = event->pos();
    if(event->buttons() & Qt::MiddleButton) {
        moveBy(event->pos() - mLastMouseEventPos);
        
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
    mLastMouseEventPos = event->pos();
    mFirstMouseMove = false;

    callUpdateSchedulers();
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0) {
        scale(1.2, event->posF());
    } else {
        scale(0.8, event->posF());
    }
    mVisibleHeight = mCombinedTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCombinedTransformMatrix.m11()*mWidth;
    
    callUpdateSchedulers();
}

void Canvas::mouseDoubleClickEvent(QMouseEvent *event)
{
    mDoubleClick = true;
    BoundingBox *boxAt = mCurrentBoxesGroup->getBoxAt(event->pos());
    if(boxAt == NULL) {
        if(mCurrentBoxesGroup != this) {
            setCurrentBoxesGroup((BoxesGroup*) mCurrentBoxesGroup->getParent());
        }
    } else {
        if(boxAt->isGroup()) {
            setCurrentBoxesGroup((BoxesGroup*) boxAt);
        } else if(mCurrentMode == MOVE_PATH) {
            setCanvasMode(MOVE_PATH);
        } else if(mCurrentMode == MOVE_PATH) {
            setCanvasMode(MOVE_PATH);
        }
    }
    
    callUpdateSchedulers();
}
