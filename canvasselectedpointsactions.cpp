#include "canvas.h"
#include "Animators/singlepathanimator.h"

void Canvas::connectPoints() {
    QList<PathPoint*> selectedPathPoints;
    foreach(MovablePoint *point, mSelectedPoints) {
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
        foreach(MovablePoint *point, mSelectedPoints) {
            if(point->isPathPoint()) {
                PathPoint *nextPoint = ((PathPoint*)point)->getNextPoint();
                if(nextPoint == NULL) continue;
                if(nextPoint->isSelected()) {
                    selectedPathPoints.append( (PathPoint*) point);
                }
            }
        }
        foreach(PathPoint *point, selectedPathPoints) {
            PathPoint *secondPoint = point->getNextPoint();
            if(point->getParentPath() ==
                    secondPoint->getParentPath()) {
                point->getParentPath()->
                        disconnectPoints(point, secondPoint);
            }
        }
}

void Canvas::mergePoints() {
    QList<PathPoint*> selectedPathPoints;
    foreach(MovablePoint *point, mSelectedPoints) {
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
    foreach(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            ( (PathPoint*)point)->setCtrlsMode(mode);
        }
    }
}

void Canvas::makeSelectedPointsSegmentsCurves() {
    QList<PathPoint*> selectedPathPoints;
    foreach(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            selectedPathPoints.append( (PathPoint*) point);
        }
    }
    foreach(PathPoint *selectedPoint,
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
    foreach(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            selectedPathPoints.append( (PathPoint*) point);
        }
    }
    foreach(PathPoint *selectedPoint,
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
        QList<BoundingBox*> parentBoxes;
        foreach(MovablePoint *point, mSelectedPoints) {
            point->finishTransform();
            BoundingBox *parentBox = point->getParent();
            if(parentBoxes.contains(parentBox) ) continue;
            parentBoxes << parentBox;
        }
        foreach(BoundingBox *parentBox, parentBoxes) {
            parentBox->finishAllPointsTransform();
        }
    } else {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->finishTransform();
        }
    }
}

void Canvas::startSelectedPointsTransform() {
    if(isRecordingAllPoints() ) {
        QList<BoundingBox*> parentBoxes;
        foreach(MovablePoint *point, mSelectedPoints) {
            point->startTransform();
            BoundingBox *parentBox = point->getParent();
            if(parentBoxes.contains(parentBox) ) continue;
            parentBoxes << parentBox;
        }
        foreach(BoundingBox *parentBox, parentBoxes) {
            parentBox->startAllPointsTransform();
        }
    } else {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->startTransform();
        }
    }
}

void Canvas::cancelSelectedPointsTransform() {
    foreach(MovablePoint *point, mSelectedPoints) {
        point->cancelTransform();
    }
}

void Canvas::moveSelectedPointsByAbs(QPointF by, bool startTransform) {
    if(startTransform) {
        startSelectedPointsTransform();
        foreach(MovablePoint *point, mSelectedPoints) {
            point->moveByAbs(by);
        }
    } else {
        foreach(MovablePoint *point, mSelectedPoints) {
            point->moveByAbs(by);
        }
    }
}

void Canvas::selectAndAddContainedPointsToSelection(QRectF absRect) {
    foreach(BoundingBox *box, mSelectedBoxes) {
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
        foreach(MovablePoint *point, mSelectedPoints) {
            pointsToDeselect << point;
        }
        foreach(MovablePoint *point, pointsToDeselect) {
            removePointFromSelection(point);
        }
    }
}

void Canvas::removeSelectedPointsApproximateAndClearList() {
    foreach(MovablePoint *point, mSelectedPoints) {
        point->deselect();
        point->removeApproximate();
    }
    mSelectedPoints.clear(); schedulePivotUpdate();
}

void Canvas::removeSelectedPointsAndClearList()
{
    foreach(MovablePoint *point, mSelectedPoints) {
        point->deselect();
        point->removeFromVectorPath();
    }
    mSelectedPoints.clear(); schedulePivotUpdate();
}

void Canvas::clearPointsSelection()
{
    foreach(MovablePoint *point, mSelectedPoints) {
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
    int count = mSelectedPoints.length();
    foreach(MovablePoint *point, mSelectedPoints) {
        posSum += point->getAbsolutePos();
    }
    return posSum/count;
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
    if(mLocalPivot || !mGlobalPivotVisible) {
        if(startTrans) {
            foreach(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->saveTransformPivotAbsPos(point->getAbsolutePos());
                point->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            foreach(MovablePoint *point, mSelectedPoints) {
                point->rotateRelativeToSavedPivot(rotBy);
            }
        }
    } else {
        if(startTrans) {
            foreach(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->saveTransformPivotAbsPos(absOrigin);
                point->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            foreach(MovablePoint *point, mSelectedPoints) {
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
    if(mLocalPivot || !mGlobalPivotVisible) {
        if(startTrans) {
            foreach(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->saveTransformPivotAbsPos(point->getAbsolutePos());
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy );
            }
        } else {
            foreach(MovablePoint *point, mSelectedPoints) {
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        }
    } else {
        if(startTrans) {
            foreach(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->saveTransformPivotAbsPos(absOrigin);
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        } else {
            foreach(MovablePoint *point, mSelectedPoints) {
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
