#include "canvas.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "MovablePoints/nodepoint.h"
#include "Animators/pathanimator.h"
#include "MovablePoints/smartnodepoint.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "MovablePoints/pathpointshandler.h"

void Canvas::connectPoints() {
    QList<SmartNodePoint*> selectedEndPts;
    for(const auto& point : mSelectedPoints_d) {
        if(point->isSmartNodePoint()) {
            auto asNodePt = GetAsPtr(point, SmartNodePoint);
            if(asNodePt->isEndPoint()) {
                selectedEndPts.append(asNodePt);
            }
        }
    }
    if(selectedEndPts.count() == 2) {
        const auto point1 = selectedEndPts.first();
        const auto point2 = selectedEndPts.last();
        const auto node1 = point1->getTargetNode();
        const auto node2 = point2->getTargetNode();

        const auto handler = point1->getHandler();
        const bool success = point2->actionConnectToNormalPoint(point1);
        if(success) {
            clearPointsSelection();
            const int targetId1 = node1->getNodeId();
            const auto sel1 = handler->getPointWithId(targetId1);
            addPointToSelection(sel1);
            const int targetId2 = node2->getNodeId();
            const auto sel2 = handler->getPointWithId(targetId2);
            addPointToSelection(sel2);
        }
    }
}

void Canvas::disconnectPoints() {
    for(const auto& point : mSelectedPoints_d) {
        if(point->isSmartNodePoint()) {
            auto asNodePt = GetAsPtr(point, SmartNodePoint);
            const auto nextPoint = asNodePt->getNextPoint();
            if(!nextPoint) continue;
            if(nextPoint->isSelected()) {
                asNodePt->actionDisconnectFromNormalPoint(nextPoint);
                break;
            }
        }
    }
    clearPointsSelection();
}

void Canvas::mergePoints() {
    QList<NodePoint*> selectedNodePoints;
    for(const auto& point : mSelectedPoints_d) {
        if(point->isNodePoint()) {
            auto asNodePt = GetAsPtr(point, NodePoint);
            //if(((NodePoint*)point)->isEndPoint()) {
                selectedNodePoints.append(asNodePt);
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
    for(const auto& point : mSelectedPoints_d) {
        if(point->isNodePoint()) {
            auto asNodePt = GetAsPtr(point, NodePoint);
            asNodePt->setCtrlsMode(mode);
        } else if(point->isSmartNodePoint()) {
            auto asNodePt = GetAsPtr(point, SmartNodePoint);
            asNodePt->setCtrlsMode(mode);
        }
    }
}

void Canvas::makeSelectedPointsSegmentsCurves() {
    QList<SmartNodePoint*> selectedSNodePoints;
    for(const auto& point : mSelectedPoints_d) {
        if(point->isSmartNodePoint()) {
            const auto asNodePt = GetAsPtr(point, SmartNodePoint);
            selectedSNodePoints.append(asNodePt);
        }
    }
    for(const auto& selectedPoint : selectedSNodePoints) {
        SmartNodePoint * const nextPoint = selectedPoint->getNextPoint();
        SmartNodePoint * const prevPoint = selectedPoint->getPreviousPoint();
        if(selectedSNodePoints.contains(nextPoint))
            selectedPoint->setC2Enabled(true);
        if(selectedSNodePoints.contains(prevPoint))
            selectedPoint->setC0Enabled(true);
    }

    QList<NodePoint*> selectedNodePoints;
    for(const auto& point : mSelectedPoints_d) {
        if(point->isNodePoint()) {
            auto asNodePt = GetAsPtr(point, NodePoint);
            selectedNodePoints.append(asNodePt);
        }
    }
    for(const auto& selectedPoint : selectedNodePoints) {
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
    QList<SmartNodePoint*> selectedSNodePoints;
    for(const auto& point : mSelectedPoints_d) {
        if(point->isSmartNodePoint()) {
            auto asNodePt = GetAsPtr(point, SmartNodePoint);
            selectedSNodePoints.append(asNodePt);
        }
    }
    for(const auto& selectedPoint : selectedSNodePoints) {
        SmartNodePoint * const nextPoint = selectedPoint->getNextPoint();
        SmartNodePoint * const prevPoint = selectedPoint->getPreviousPoint();
        if(selectedSNodePoints.contains(nextPoint))
            selectedPoint->setC2Enabled(false);
        if(selectedSNodePoints.contains(prevPoint))
            selectedPoint->setC0Enabled(false);
    }

    QList<NodePoint*> selectedNodePoints;
    for(const auto& point : mSelectedPoints_d) {
        if(point->isNodePoint()) {
            auto asNodePt = GetAsPtr(point, NodePoint);
            selectedNodePoints.append(asNodePt);
        }
    }
    for(const auto& selectedPoint : selectedNodePoints) {
        NodePoint * const nextPoint = selectedPoint->getNextPoint();
        NodePoint * const prevPoint = selectedPoint->getPreviousPoint();
        if(selectedNodePoints.contains(nextPoint))
            selectedPoint->setEndCtrlPtEnabled(false);
        if(selectedNodePoints.contains(prevPoint))
            selectedPoint->setStartCtrlPtEnabled(false);
    }
}

void Canvas::finishSelectedPointsTransform() {
    for(const auto& point : mSelectedPoints_d) {
        point->finishTransform();
    }
}

void Canvas::startSelectedPointsTransform() {
    for(const auto& point : mSelectedPoints_d) {
        point->startTransform();
    }
}

void Canvas::cancelSelectedPointsTransform() {
    for(const auto& point : mSelectedPoints_d) {
        point->cancelTransform();
    }
}

void Canvas::moveSelectedPointsByAbs(const QPointF &by,
                                     const bool &startTransform) {
    if(startTransform) {
        startSelectedPointsTransform();
        for(const auto& point : mSelectedPoints_d) {
            point->moveByAbs(by);
        }
    } else {
        for(const auto& point : mSelectedPoints_d) {
            point->moveByAbs(by);
        }
    }
}

void Canvas::selectAndAddContainedPointsToSelection(const QRectF& absRect) {
    for(const auto& box : mSelectedBoxes) {
        box->selectAndAddContainedPointsToList(absRect, mSelectedPoints_d);
    }
}

void Canvas::addPointToSelection(MovablePoint* const point) {
    if(point->isSelected()) return;
    point->select();
    mSelectedPoints_d.append(point); schedulePivotUpdate();
}

void Canvas::removePointFromSelection(MovablePoint * const point) {
    point->deselect();
    mSelectedPoints_d.removeOne(point); schedulePivotUpdate();
}

void Canvas::updateSelectedPointsAfterCtrlsVisiblityChanged() {
    if(!BoxesGroup::mCtrlsAlwaysVisible) {
        QList<MovablePoint*> pointsToDeselect;
        for(const auto& point : mSelectedPoints_d) {
            pointsToDeselect << point;
        }
        for(const auto& point : pointsToDeselect) {
            removePointFromSelection(point);
        }
    }
}

void Canvas::removeSelectedPointsApproximateAndClearList() {
    for(const auto& point : mSelectedPoints_d) {
        point->deselect();
        point->removeApproximate();
    }
    mSelectedPoints_d.clear(); schedulePivotUpdate();
}

void Canvas::removeSelectedPointsAndClearList() {
    if(mIsMouseGrabbing) {
        if(!BoxesGroup::mCtrlsAlwaysVisible ||
            mSelectedPoints_d.count() == 1) {
            if(mLastPressedPoint) {
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

    for(const auto& point : mSelectedPoints_d) {
        point->deselect();
        point->removeFromVectorPath();
    }
    mSelectedPoints_d.clear(); schedulePivotUpdate();
}

void Canvas::clearPointsSelection() {
    for(const auto& point : mSelectedPoints_d) {
        if(point) point->deselect();
    }

    mSelectedPoints_d.clear();
    if(mCurrentMode == MOVE_POINT) schedulePivotUpdate();
//    if(mLastPressedPoint) {
//        mLastPressedPoint->deselect();
//        mLastPressedPoint = nullptr;
//    }
//    setCurrentEndPoint(nullptr);
}

void Canvas::clearLastPressedPoint() {
    if(mLastPressedPoint) {
        mLastPressedPoint->deselect();
        mLastPressedPoint = nullptr;
    }
}

void Canvas::clearCurrentEndPoint() {
    setCurrentEndPoint(nullptr);
}

QPointF Canvas::getSelectedPointsAbsPivotPos() {
    if(mSelectedPoints_d.isEmpty()) return QPointF(0, 0);
    QPointF posSum(0, 0);
    for(const auto& point : mSelectedPoints_d) {
        posSum += point->getAbsolutePos();
    }
    const qreal invCount = 1./mSelectedPoints_d.length();
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
            for(const auto& point : mSelectedPoints_d) {
                point->startTransform();
                point->saveTransformPivotAbsPos(point->getAbsolutePos());
                point->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            for(const auto& point : mSelectedPoints_d) {
                point->rotateRelativeToSavedPivot(rotBy);
            }
        }
    } else {
        if(startTrans) {
            for(const auto& point : mSelectedPoints_d) {
                point->startTransform();
                point->saveTransformPivotAbsPos(absOrigin);
                point->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            for(const auto& point : mSelectedPoints_d) {
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
            for(const auto& point : mSelectedPoints_d) {
                point->startTransform();
                point->saveTransformPivotAbsPos(point->getAbsolutePos());
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy );
            }
        } else {
            for(const auto& point : mSelectedPoints_d) {
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        }
    } else {
        if(startTrans) {
            for(const auto& point : mSelectedPoints_d) {
                point->startTransform();
                point->saveTransformPivotAbsPos(absOrigin);
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        } else {
            for(const auto& point : mSelectedPoints_d) {
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
        clearCurrentSmartEndPoint();
        clearLastPressedPoint();
    }
}
