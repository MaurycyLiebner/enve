#include "canvas.h"
#include "Animators/singlepathanimator.h"
#include "pathpoint.h"

void Canvas::connectPoints() {
    QList<PathPoint*> selectedPathPoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            if(((PathPoint*)point)->isEndPoint()) {
                selectedPathPoints.append( (PathPoint*) point);
            }
        }
    }
    if(selectedPathPoints.count() == 2) {
        PathPoint *firstPoint = selectedPathPoints.first();
        PathPoint *secondPoint = selectedPathPoints.last();
        if(firstPoint->getParentPath()->getParentPathAnimator() ==
                secondPoint->getParentPath()->getParentPathAnimator()) {
            firstPoint->getParentPath()->
                    connectPoints(firstPoint, secondPoint);
        }
    }
}

void Canvas::disconnectPoints() {
    QList<PathPoint*> selectedPathPoints;
        Q_FOREACH(MovablePoint *point, mSelectedPoints) {
            if(point->isPathPoint()) {
                PathPoint *nextPoint = ((PathPoint*)point)->getNextPoint();
                if(nextPoint == NULL) continue;
                if(nextPoint->isSelected()) {
                    selectedPathPoints.append( (PathPoint*) point);
                }
            }
        }
        Q_FOREACH(PathPoint *point, selectedPathPoints) {
            PathPoint *secondPoint = point->getNextPoint();
            if(secondPoint == NULL) secondPoint = point->getPreviousPoint();
            if(point->getParentPath() ==
                    secondPoint->getParentPath()) {
                point->getParentPath()->
                        disconnectPoints(point, secondPoint);
            }
        }
}

void Canvas::mergePoints() {
    QList<PathPoint*> selectedPathPoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            //if(((PathPoint*)point)->isEndPoint()) {
                selectedPathPoints.append( (PathPoint*) point);
            //}
        }
    }
    if(selectedPathPoints.count() == 2) {
        PathPoint *firstPoint = selectedPathPoints.first();
        PathPoint *secondPoint = selectedPathPoints.last();
        if(firstPoint->isEndPoint() &&
           secondPoint->isEndPoint()) {

            secondPoint->getParentPath()->connectPoints(firstPoint,
                                                  secondPoint);
        } else if(firstPoint->getNextPoint() != secondPoint &&
                  firstPoint->getPreviousPoint() != secondPoint) {
            return;
        }
        QPointF sumPos = firstPoint->getAbsolutePos() +
                secondPoint->getAbsolutePos();
        bool firstWasPrevious = firstPoint ==
                secondPoint->getPreviousPoint();

        secondPoint->startTransform();
        secondPoint->moveToAbs(sumPos/2);
        if(firstWasPrevious) {
            secondPoint->moveStartCtrlPtToAbsPos(
                        firstPoint->getStartCtrlPtAbsPos());
        } else {
            secondPoint->moveEndCtrlPtToAbsPos(
                        firstPoint->getEndCtrlPtAbsPos());
        }
        secondPoint->finishTransform();

        firstPoint->removeFromVectorPath();
    }
}

void Canvas::setPointCtrlsMode(CtrlsMode mode) {
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            ( (PathPoint*)point)->setCtrlsMode(mode);
        }
    }
}

void Canvas::makeSelectedPointsSegmentsCurves() {
    QList<PathPoint*> selectedPathPoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            selectedPathPoints.append( (PathPoint*) point);
        }
    }
    Q_FOREACH(PathPoint *selectedPoint,
            selectedPathPoints) {
        PathPoint *nextPoint = selectedPoint->getNextPoint();
        PathPoint *prevPoint = selectedPoint->getPreviousPoint();
        if(selectedPathPoints.contains(nextPoint)) {
            selectedPoint->setEndCtrlPtEnabled(true);
        }
        if(selectedPathPoints.contains(prevPoint)) {
            selectedPoint->setStartCtrlPtEnabled(true);
        }
    }
}

void Canvas::makeSelectedPointsSegmentsLines() {
    QList<PathPoint*> selectedPathPoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            selectedPathPoints.append( (PathPoint*) point);
        }
    }
    Q_FOREACH(PathPoint *selectedPoint,
            selectedPathPoints) {
        PathPoint *nextPoint = selectedPoint->getNextPoint();
        PathPoint *prevPoint = selectedPoint->getPreviousPoint();
        if(selectedPathPoints.contains(nextPoint)) {
            selectedPoint->setEndCtrlPtEnabled(false);
        }
        if(selectedPathPoints.contains(prevPoint)) {
            selectedPoint->setStartCtrlPtEnabled(false);
        }
    }
}

void Canvas::finishSelectedPointsTransform() {
    if(isRecordingAllPoints() ) {
        QList<SinglePathAnimator*> separatePaths;
        Q_FOREACH(MovablePoint *point, mSelectedPoints) {
            if(point->isPathPoint()) {
                SinglePathAnimator *sPath = ((PathPoint*)point)->getParentPath();
                if(separatePaths.contains(sPath)) continue;
                separatePaths << sPath;
            } else {
                point->finishTransform();
            }
        }
        Q_FOREACH(SinglePathAnimator *path, separatePaths) {
            path->finishAllPointsTransform();
        }
    } else {
        Q_FOREACH(MovablePoint *point, mSelectedPoints) {
            point->finishTransform();
        }
    }
}

void Canvas::startSelectedPointsTransform() {
    if(isRecordingAllPoints() ) {
        QList<SinglePathAnimator*> separatePaths;
        Q_FOREACH(MovablePoint *point, mSelectedPoints) {
            if(point->isPathPoint()) {
                SinglePathAnimator *sPath = ((PathPoint*)point)->getParentPath();
                if(separatePaths.contains(sPath)) continue;
                separatePaths << sPath;
            } else {
                point->startTransform();
            }
        }
        Q_FOREACH(SinglePathAnimator *path, separatePaths) {
            path->startAllPointsTransform();
        }
    } else {
        Q_FOREACH(MovablePoint *point, mSelectedPoints) {
            point->startTransform();
        }
    }
}

void Canvas::cancelSelectedPointsTransform() {
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        point->cancelTransform();
    }
}

void Canvas::moveSelectedPointsByAbs(const QPointF &by,
                                     const bool &startTransform) {
    if(startTransform) {
        startSelectedPointsTransform();
        Q_FOREACH(MovablePoint *point, mSelectedPoints) {
            point->moveByAbs(by);
        }
    } else {
        Q_FOREACH(MovablePoint *point, mSelectedPoints) {
            point->moveByAbs(by);
        }
    }
}

void Canvas::selectAndAddContainedPointsToSelection(QRectF absRect) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->selectAndAddContainedPointsToList(absRect, &mSelectedPoints);
    }
}

void Canvas::addPointToSelection(MovablePoint *point) {
    if(point->isSelected()) {
        return;
    }
    point->select();
    mSelectedPoints.append(point); schedulePivotUpdate();
}

void Canvas::removePointFromSelection(MovablePoint *point) {
    point->deselect();
    mSelectedPoints.removeOne(point); schedulePivotUpdate();
}

void Canvas::updateSelectedPointsAfterCtrlsVisiblityChanged() {
    if(!BoxesGroup::mCtrlsAlwaysVisible) {
        QList<MovablePoint*> pointsToDeselect;
        Q_FOREACH(MovablePoint *point, mSelectedPoints) {
            pointsToDeselect << point;
        }
        Q_FOREACH(MovablePoint *point, pointsToDeselect) {
            removePointFromSelection(point);
        }
    }
}

void Canvas::removeSelectedPointsApproximateAndClearList() {
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        point->deselect();
        point->removeApproximate();
    }
    mSelectedPoints.clear(); schedulePivotUpdate();
}

void Canvas::removeSelectedPointsAndClearList()
{
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        point->deselect();
        point->removeFromVectorPath();
    }
    mSelectedPoints.clear(); schedulePivotUpdate();
}

void Canvas::clearPointsSelection()
{
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        point->deselect();
    }
    mSelectedPoints.clear();
    if(mCurrentMode == MOVE_POINT) {
        schedulePivotUpdate();
    }
//    if(mLastPressedPoint != NULL) {
//        mLastPressedPoint->deselect();
//        mLastPressedPoint = NULL;
//    }
//    setCurrentEndPoint(NULL);
}

void Canvas::clearLastPressedPoint() {
    if(mLastPressedPoint != NULL) {
        mLastPressedPoint->deselect();
        mLastPressedPoint = NULL;
    }
}

void Canvas::clearCurrentEndPoint() {
    setCurrentEndPoint(NULL);
}

QPointF Canvas::getSelectedPointsAbsPivotPos() {
    if(mSelectedPoints.isEmpty()) return QPointF(0., 0.);
    QPointF posSum = QPointF(0., 0.);
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        posSum += point->getAbsolutePos();
    }
    qreal invCount = 1./mSelectedPoints.length();
    return posSum*invCount;
}

bool Canvas::isPointsSelectionEmpty() {
    return mSelectedPoints.isEmpty();
}

int Canvas::getPointsSelectionCount() {
    return mSelectedPoints.length();
}

void Canvas::rotateSelectedPointsBy(const qreal &rotBy,
                                    const QPointF &absOrigin,
                                    const bool &startTrans) {
    if(mSelectedPoints.isEmpty()) return;
    if(mLocalPivot) {
        if(startTrans) {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->saveTransformPivotAbsPos(point->getAbsolutePos());
                point->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->rotateRelativeToSavedPivot(rotBy);
            }
        }
    } else {
        if(startTrans) {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->saveTransformPivotAbsPos(absOrigin);
                point->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->rotateRelativeToSavedPivot(rotBy);
            }
        }
    }
}

void Canvas::scaleSelectedPointsBy(const qreal &scaleXBy,
                                   const qreal &scaleYBy,
                                   const QPointF &absOrigin,
                                   const bool &startTrans) {
    if(mSelectedPoints.isEmpty()) return;
    if(mLocalPivot) {
        if(startTrans) {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->saveTransformPivotAbsPos(point->getAbsolutePos());
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy );
            }
        } else {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        }
    } else {
        if(startTrans) {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->saveTransformPivotAbsPos(absOrigin);
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        } else {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        }
    }
}

void Canvas::clearPointsSelectionOrDeselect() {
    if(mSelectedPoints.isEmpty() ) {
        deselectAllBoxes();
    } else {
        clearPointsSelection();
        clearCurrentEndPoint();
        clearLastPressedPoint();
    }
}
