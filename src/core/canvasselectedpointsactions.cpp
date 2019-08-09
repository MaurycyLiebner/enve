#include "canvas.h"
#include "MovablePoints/smartnodepoint.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "MovablePoints/pathpointshandler.h"

void Canvas::connectPoints() {
    QList<SmartNodePoint*> selectedEndPts;
    for(const auto& point : mSelectedPoints_d) {
        if(point->isSmartNodePoint()) {
            auto asNodePt = static_cast<SmartNodePoint*>(point);
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
            const auto sel1 = handler->getPointWithId<SmartNodePoint>(targetId1);
            addPointToSelection(sel1);
            const int targetId2 = node2->getNodeId();
            const auto sel2 = handler->getPointWithId<SmartNodePoint>(targetId2);
            addPointToSelection(sel2);
        }
    }
}

void Canvas::disconnectPoints() {
    for(const auto& point : mSelectedPoints_d) {
        if(point->isSmartNodePoint()) {
            auto asNodePt = static_cast<SmartNodePoint*>(point);
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
    QList<SmartNodePoint*> selectedNodePoints;
    for(const auto& point : mSelectedPoints_d) {
        if(point->isSmartNodePoint()) {
            const auto asNodePt = static_cast<SmartNodePoint*>(point);
            selectedNodePoints.append(asNodePt);
        }
    }
    if(selectedNodePoints.count() == 2) {
        std::sort(selectedNodePoints.begin(), selectedNodePoints.end(),
                  [](const SmartNodePoint * const v1,
                     const SmartNodePoint * const v2) {
            return v1->getNodeId() < v2->getNodeId();
        });
        const auto firstPoint = selectedNodePoints.first();
        const auto secondPoint = selectedNodePoints.last();
        bool vailable = false;
        vailable = vailable || (firstPoint->isEndPoint() &&
                                secondPoint->isEndPoint());
        const bool ends = firstPoint->isEndPoint() && secondPoint->isEndPoint();
        const bool neigh = firstPoint->getPreviousPoint() == secondPoint ||
                           firstPoint->getNextPoint() == secondPoint;
        if(!ends && !neigh) return;
        if(ends) {
            connectPoints();
            mergePoints();
            return;
        }
        removePointFromSelection(secondPoint);
        firstPoint->actionMergeWithNormalPoint(secondPoint);
    }
}

void Canvas::setPointCtrlsMode(const CtrlsMode mode) {
    for(const auto& point : mSelectedPoints_d) {
        if(point->isSmartNodePoint()) {
            auto asNodePt = static_cast<SmartNodePoint*>(point);
            asNodePt->setCtrlsMode(mode);
        }
    }
}

void Canvas::makeSelectedPointsSegmentsCurves() {
    QList<SmartNodePoint*> selectedSNodePoints;
    for(const auto& point : mSelectedPoints_d) {
        if(point->isSmartNodePoint()) {
            const auto asNodePt = static_cast<SmartNodePoint*>(point);
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
}

void Canvas::makeSelectedPointsSegmentsLines() {
    QList<SmartNodePoint*> selectedSNodePoints;
    for(const auto& point : mSelectedPoints_d) {
        if(point->isSmartNodePoint()) {
            auto asNodePt = static_cast<SmartNodePoint*>(point);
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
                                     const bool startTransform) {
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
        box->selectAndAddContainedPointsToList(absRect, mSelectedPoints_d,
                                               mCurrentMode);
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

void Canvas::removeSelectedPointsAndClearList() {
    if(mPressedPoint && mPressedPoint->isCtrlPoint()) {
        mPressedPoint->cancelTransform();
        mPressedPoint->deselect();
        mPressedPoint->remove();
        mSelectedPoints_d.removeOne(mPressedPoint);
        schedulePivotUpdate();
        return;
    }

    for(const auto& point : mSelectedPoints_d) {
        point->deselect();
        point->remove();
    }
    mSelectedPoints_d.clear(); schedulePivotUpdate();
}

void Canvas::clearPointsSelection() {
    for(const auto& point : mSelectedPoints_d) {
        if(point) point->deselect();
    }

    mSelectedPoints_d.clear();
    if(mCurrentMode == CanvasMode::pointTransform) schedulePivotUpdate();
//    if(mLastPressedPoint) {
//        mLastPressedPoint->deselect();
//        mLastPressedPoint = nullptr;
//    }
//    setCurrentEndPoint(nullptr);
}

void Canvas::clearLastPressedPoint() {
    if(mPressedPoint) {
        mPressedPoint->deselect();
        mPressedPoint = nullptr;
    }
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

bool Canvas::isPointSelectionEmpty() const {
    return mSelectedPoints_d.isEmpty();
}

int Canvas::getPointsSelectionCount() const {
    return mSelectedPoints_d.length();
}

void Canvas::rotateSelectedPointsBy(const qreal rotBy,
                                    const QPointF &absOrigin,
                                    const bool startTrans) {
    if(mSelectedPoints_d.isEmpty()) return;
    if(startTrans) {
        if(mDocument.fLocalPivot) {
            for(const auto& point : mSelectedPoints_d) {
                point->startTransform();
                point->saveTransformPivotAbsPos(point->getAbsolutePos());
                point->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            for(const auto& point : mSelectedPoints_d) {
                point->startTransform();
                point->saveTransformPivotAbsPos(absOrigin);
                point->rotateRelativeToSavedPivot(rotBy);
            }
        }
    } else {
        for(const auto& point : mSelectedPoints_d) {
            point->rotateRelativeToSavedPivot(rotBy);
        }
    }
}

void Canvas::scaleSelectedPointsBy(const qreal scaleXBy,
                                   const qreal scaleYBy,
                                   const QPointF &absOrigin,
                                   const bool startTrans) {
    if(mSelectedPoints_d.isEmpty()) return;
    if(startTrans) {
        if(mDocument.fLocalPivot) {
            for(const auto& point : mSelectedPoints_d) {
                point->startTransform();
                point->saveTransformPivotAbsPos(point->getAbsolutePos());
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        } else {
            for(const auto& point : mSelectedPoints_d) {
                point->startTransform();
                point->saveTransformPivotAbsPos(absOrigin);
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        }
    } else {
        for(const auto& point : mSelectedPoints_d) {
            point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
        }
    }
}

void Canvas::clearPointsSelectionOrDeselect() {
    if(mSelectedPoints_d.isEmpty() ) {
        deselectAllBoxes();
    } else {
        clearPointsSelection();
        clearCurrentSmartEndPoint();
        clearLastPressedPoint();
    }
}
