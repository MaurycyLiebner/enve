#include "canvas.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "nodepoint.h"
#include "Animators/pathanimator.h"

void Canvas::connectPoints() {
    QList<NodePoint*> selectedNodePoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
        if(point->isNodePoint()) {
            if(((NodePoint*)point)->isEndPoint()) {
                selectedNodePoints.append( (NodePoint*) point);
            }
        }
    }

    if(selectedNodePoints.count() == 2) {
        NodePoint *firstPoint = selectedNodePoints.first();
        NodePoint *secondPoint = selectedNodePoints.last();
        if(!firstPoint->isEndPoint() || !secondPoint->isEndPoint()) return;
        clearPointsSelection();

        VectorPathAnimator *firstSinglePath = firstPoint->getParentPath();
        VectorPathAnimator *secondSinglePath = secondPoint->getParentPath();
        PathAnimator *firstParentPath = firstSinglePath->getParentPathAnimator();
        PathAnimator *secondParentPath = secondSinglePath->getParentPathAnimator();

        int nodeToSelectId = firstPoint->getNodeId();
        VectorPathAnimator *newSinglePath = nullptr;
        if(firstParentPath == secondParentPath) {
            if(firstSinglePath == secondSinglePath) {
                firstSinglePath->connectPoints(firstPoint, secondPoint);
            } else {
                if(firstPoint->isSeparateNodePoint()) {
                    firstSinglePath->revertAllPointsForAllKeys();
                    nodeToSelectId = firstSinglePath->getNodeCount() - 1 -
                            nodeToSelectId;
                }
                if(!secondPoint->isSeparateNodePoint()) {
                    secondSinglePath->revertAllPointsForAllKeys();
                }
                newSinglePath = firstSinglePath->connectWith(secondSinglePath);
            }
        } else {
            if(firstPoint->isSeparateNodePoint()) {
                firstSinglePath->revertAllPointsForAllKeys();
                nodeToSelectId = firstSinglePath->getNodeCount() - 1 -
                        nodeToSelectId;
            }
            if(!secondPoint->isSeparateNodePoint()) {
                secondSinglePath->revertAllPointsForAllKeys();
            }
            QMatrix firstMatrix =
                    firstParentPath->getCombinedTransform();
            QMatrix secondMatrix =
                    secondParentPath->getCombinedTransform();
            QMatrix effectiveMatrix =
                    secondMatrix*firstMatrix.inverted();
            secondSinglePath->applyTransformToPoints(effectiveMatrix);
            firstParentPath->addSinglePathAnimator(secondSinglePath);
            secondSinglePath->removeFromParent();
            secondSinglePath->setParentPath(firstParentPath);

            newSinglePath = firstSinglePath->connectWith(secondSinglePath);
        }
        if(newSinglePath != nullptr) {
            NodePoint *nodePt1 = newSinglePath->getNodePtWithNodeId(nodeToSelectId);
            NodePoint *nodePt2 = newSinglePath->getNodePtWithNodeId(nodeToSelectId + 1);
            addPointToSelection(nodePt1);
            addPointToSelection(nodePt2);
        }
    }
}

void Canvas::disconnectPoints() {
    QList<NodePoint*> selectedNodePoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
        if(point->isNodePoint()) {
            NodePoint *nextPoint = ((NodePoint*)point)->getNextPoint();
            if(nextPoint == nullptr) continue;
            if(nextPoint->isSelected()) {
                selectedNodePoints.append((NodePoint*)point);
            }
        }
    }
    if(selectedNodePoints.count() != 1) return;
    clearPointsSelection();
    Q_FOREACH(NodePoint *point, selectedNodePoints) {
        NodePoint *secondPoint = point->getNextPoint();
        VectorPathAnimator *parentPath = point->getParentPath();
        bool wasClosed = parentPath->isClosed();
        parentPath->disconnectPoints(point, secondPoint);
        if(wasClosed) {
            addPointToSelection(parentPath->getNodePtWithNodeId(0));
            addPointToSelection(parentPath->getNodePtWithNodeId(
                                    parentPath->getNodeCount() - 1));
        }
    }
}

//void Canvas::mergePoints() {
//    QList<NodePoint*> selectedNodePoints;
//    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
//        if(point->isNodePoint()) {
//            if(((NodePoint*)point)->isEndPoint()) {
//                selectedNodePoints.append( (NodePoint*) point);
//            }
//        }
//    }
//    clearPointsSelection();

//    if(selectedNodePoints.count() == 2) {
//        bool vailable = false;
//        vailable = vailable || (firstPoint->isEndPoint() &&
//                                secondPoint->isEndPoint());
//        vailable = vailable || (firstPoint->getPreviousPoint() == secondPoint);
//        vailable = vailable || (firstPoint->getNextPoint() == secondPoint);
//        if(!vailable) return;
//        NodePoint *firstPoint = selectedNodePoints.first();
//        NodePoint *secondPoint = selectedNodePoints.last();

//        QPointF sumPos = firstPoint->getAbsolutePos() +
//                secondPoint->getAbsolutePos();
//        bool firstWasPrevious = firstPoint ==
//                secondPoint->getPreviousPoint();

//        secondPoint->startTransform();
//        secondPoint->moveToAbs(sumPos/2);
//        if(firstWasPrevious) {
//            secondPoint->moveStartCtrlPtToAbsPos(
//                        firstPoint->getStartCtrlPtAbsPos());
//        } else {
//            secondPoint->moveEndCtrlPtToAbsPos(
//                        firstPoint->getEndCtrlPtAbsPos());
//        }
//        secondPoint->finishTransform();

//        NodePoint *firstPointT;
//        firstPointT->removeFromVectorPath();

//        VectorPathAnimator *firstSinglePath = firstPoint->getParentPath();
//        VectorPathAnimator *secondSinglePath = secondPoint->getParentPath();
//        PathAnimator *firstParentPath = firstSinglePath->getParentPathAnimator();
//        PathAnimator *secondParentPath = secondSinglePath->getParentPathAnimator();
//        if(firstParentPath == secondParentPath) {
//            if(firstSinglePath == secondSinglePath) {
//                firstSinglePath->connectPoints(firstPoint, secondPoint);
//            } else {
//                if(firstPoint->isSeparateNodePoint()) {
//                    firstSinglePath->revertAllPointsForAllKeys();
//                }
//                if(!secondPoint->isSeparateNodePoint()) {
//                    secondSinglePath->revertAllPointsForAllKeys();
//                }
//                firstSinglePath->connectWith(secondSinglePath);
//            }
//        } else {
//            if(firstPoint->isSeparateNodePoint()) {
//                firstSinglePath->revertAllPointsForAllKeys();
//            }
//            if(!secondPoint->isSeparateNodePoint()) {
//                secondSinglePath->revertAllPointsForAllKeys();
//            }
//            QMatrix firstMatrix =
//                    firstParentPath->getCombinedTransform();
//            QMatrix secondMatrix =
//                    secondParentPath->getCombinedTransform();
//            QMatrix effectiveMatrix =
//                    secondMatrix*firstMatrix.inverted();
//            secondSinglePath->applyTransformToPoints(effectiveMatrix);
//            firstParentPath->addSinglePathAnimator(secondSinglePath);
//            secondSinglePath->removeFromParent();
//            secondSinglePath->setParentPath(firstParentPath);

//            firstSinglePath->connectWith(secondSinglePath);
//        }
//    }
//}

void Canvas::mergePoints() {
    QList<NodePoint*> selectedNodePoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
        if(point->isNodePoint()) {
            //if(((NodePoint*)point)->isEndPoint()) {
                selectedNodePoints.append( (NodePoint*) point);
            //}
        }
    }
    if(selectedNodePoints.count() == 2) {
        NodePoint *firstPoint = selectedNodePoints.first();
        NodePoint *secondPoint = selectedNodePoints.last();
        bool vailable = false;
        vailable = vailable || (firstPoint->isEndPoint() &&
                                secondPoint->isEndPoint());
        vailable = vailable || (firstPoint->getPreviousPoint() == secondPoint);
        vailable = vailable || (firstPoint->getNextPoint() == secondPoint);
        if(!vailable) return;
        clearPointsSelection();

        if(firstPoint->getPreviousPoint() != secondPoint &&
           firstPoint->getNextPoint() != secondPoint) {
            addPointToSelection(firstPoint);
            addPointToSelection(secondPoint);
            connectPoints();
            mergePoints();
            return;
        }
        int selectNodeId = qMin(firstPoint->getNodeId(),
                                secondPoint->getNodeId());
        VectorPathAnimator *parentPath = firstPoint->getParentPath();
        parentPath->mergeNodes(firstPoint->getNodeId(),
                               secondPoint->getNodeId());
        addPointToSelection(parentPath->getNodePtWithNodeId(selectNodeId));
    }
}

void Canvas::setPointCtrlsMode(const CtrlsMode& mode) {
    Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
        if(point->isNodePoint()) {
            ((NodePoint*)point)->setCtrlsMode(mode);
        }
    }
}

void Canvas::makeSelectedPointsSegmentsCurves() {
    QList<NodePoint*> selectedNodePoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
        if(point->isNodePoint()) {
            selectedNodePoints.append((NodePoint*)point);
        }
    }
    Q_FOREACH(NodePoint *selectedPoint,
            selectedNodePoints) {
        NodePoint *nextPoint = selectedPoint->getNextPoint();
        NodePoint *prevPoint = selectedPoint->getPreviousPoint();
        if(selectedNodePoints.contains(nextPoint)) {
            selectedPoint->setEndCtrlPtEnabled(true);
        }
        if(selectedNodePoints.contains(prevPoint)) {
            selectedPoint->setStartCtrlPtEnabled(true);
        }
    }
}

void Canvas::makeSelectedPointsSegmentsLines() {
    QList<NodePoint*> selectedNodePoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
        if(point->isNodePoint()) {
            selectedNodePoints.append( (NodePoint*) point);
        }
    }
    Q_FOREACH(NodePoint *selectedPoint,
            selectedNodePoints) {
        NodePoint *nextPoint = selectedPoint->getNextPoint();
        NodePoint *prevPoint = selectedPoint->getPreviousPoint();
        if(selectedNodePoints.contains(nextPoint)) {
            selectedPoint->setEndCtrlPtEnabled(false);
        }
        if(selectedNodePoints.contains(prevPoint)) {
            selectedPoint->setStartCtrlPtEnabled(false);
        }
    }
}

void Canvas::finishSelectedPointsTransform() {
    Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
        point->finishTransform();
    }
}

void Canvas::startSelectedPointsTransform() {
    Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
        point->startTransform();
    }
}

void Canvas::cancelSelectedPointsTransform() {
    Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
        point->cancelTransform();
    }
}

void Canvas::moveSelectedPointsByAbs(const QPointF &by,
                                     const bool &startTransform) {
    if(startTransform) {
        startSelectedPointsTransform();
        Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
            point->moveByAbs(by);
        }
    } else {
        Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
            point->moveByAbs(by);
        }
    }
}

void Canvas::selectAndAddContainedPointsToSelection(const QRectF& absRect) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->selectAndAddContainedPointsToList(absRect, &mSelectedPoints_d);
    }
}

void Canvas::addPointToSelection(const MovablePoint& point) {
    if(point->isSelected()) {
        return;
    }
    point->select();
    mSelectedPoints_d.append(point); schedulePivotUpdate();
}

void Canvas::removePointFromSelection(MovablePoint *point) {
    point->deselect();
    mSelectedPoints_d.removeOne(point); schedulePivotUpdate();
}

void Canvas::updateSelectedPointsAfterCtrlsVisiblityChanged() {
    if(!BoxesGroup::mCtrlsAlwaysVisible) {
        QList<MovablePointSPtr> pointsToDeselect;
        Q_FOREACH(const MovablePointSPtr& point, mSelectedPoints_d) {
            pointsToDeselect << point;
        }
        Q_FOREACH(const MovablePointSPtr& point, pointsToDeselect) {
            removePointFromSelection(point);
        }
    }
}

void Canvas::removeSelectedPointsApproximateAndClearList() {
    Q_FOREACH(const MovablePointSPtr& point, mSelectedPoints_d) {
        point->deselect();
        point->removeApproximate();
    }
    mSelectedPoints_d.clear(); schedulePivotUpdate();
}

void Canvas::removeSelectedPointsAndClearList() {
    if(mIsMouseGrabbing) {
        if(!BoxesGroup::mCtrlsAlwaysVisible ||
            mSelectedPoints_d.count() == 1) {
            if(mLastPressedPoint != nullptr) {
                if(mLastPressedPoint->isCtrlPoint()) {
                    mLastPressedPoint->cancelTransform();
                    mLastPressedPoint->deselect();
                    mLastPressedPoint->removeFromVectorPath();
                    mSelectedPoints_d.removeOne(mLastPressedPoint);
                    schedulePivotUpdate();
                    return;
                }
            }
        }
    }

    Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
        point->deselect();
        point->removeFromVectorPath();
    }
    mSelectedPoints_d.clear(); schedulePivotUpdate();
}

void Canvas::clearPointsSelection() {
    Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
        point->deselect();
    }
    mSelectedPoints_d.clear();
    if(mCurrentMode == MOVE_POINT) {
        schedulePivotUpdate();
    }
//    if(mLastPressedPoint != nullptr) {
//        mLastPressedPoint->deselect();
//        mLastPressedPoint = nullptr;
//    }
//    setCurrentEndPoint(nullptr);
}

void Canvas::clearLastPressedPoint() {
    if(mLastPressedPoint != nullptr) {
        mLastPressedPoint->deselect();
        mLastPressedPoint = nullptr;
    }
}

void Canvas::clearCurrentEndPoint() {
    setCurrentEndPoint(nullptr);
}

QPointF Canvas::getSelectedPointsAbsPivotPos() {
    if(mSelectedPoints_d.isEmpty()) return QPointF(0., 0.);
    QPointF posSum = QPointF(0., 0.);
    Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
        posSum += point->getAbsolutePos();
    }
    qreal invCount = 1./mSelectedPoints_d.length();
    return posSum*invCount;
}

bool Canvas::isPointsSelectionEmpty() {
    return mSelectedPoints_d.isEmpty();
}

int Canvas::getPointsSelectionCount() {
    return mSelectedPoints_d.length();
}

void Canvas::rotateSelectedPointsBy(const qreal &rotBy,
                                    const QPointF &absOrigin,
                                    const bool &startTrans) {
    if(mSelectedPoints_d.isEmpty()) return;
    if(mLocalPivot) {
        if(startTrans) {
            Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
                point->startTransform();
                point->saveTransformPivotAbsPos(point->getAbsolutePos());
                point->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
                point->rotateRelativeToSavedPivot(rotBy);
            }
        }
    } else {
        if(startTrans) {
            Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
                point->startTransform();
                point->saveTransformPivotAbsPos(absOrigin);
                point->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
                point->rotateRelativeToSavedPivot(rotBy);
            }
        }
    }
}

void Canvas::scaleSelectedPointsBy(const qreal &scaleXBy,
                                   const qreal &scaleYBy,
                                   const QPointF &absOrigin,
                                   const bool &startTrans) {
    if(mSelectedPoints_d.isEmpty()) return;
    if(mLocalPivot) {
        if(startTrans) {
            Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
                point->startTransform();
                point->saveTransformPivotAbsPos(point->getAbsolutePos());
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy );
            }
        } else {
            Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        }
    } else {
        if(startTrans) {
            Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
                point->startTransform();
                point->saveTransformPivotAbsPos(absOrigin);
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        } else {
            Q_FOREACH(MovablePoint *point, mSelectedPoints_d) {
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        }
    }
}

void Canvas::clearPointsSelectionOrDeselect() {
    if(mSelectedPoints_d.isEmpty() ) {
        deselectAllBoxes();
    } else {
        clearPointsSelection();
        clearCurrentEndPoint();
        clearLastPressedPoint();
    }
}
