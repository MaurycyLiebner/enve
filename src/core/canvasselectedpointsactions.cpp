// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "canvas.h"
#include "MovablePoints/smartnodepoint.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "MovablePoints/pathpointshandler.h"
#include "Private/document.h"

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
    const auto adder = [this](MovablePoint* const pt) {
        addPointToSelection(pt);
    };
    for(const auto& box : mSelectedBoxes) {
        box->selectAndAddContainedPointsToList(absRect, adder, mCurrentMode);
    }
}

void Canvas::addPointToSelection(MovablePoint* const point) {
    if(point->isSelected()) return;
    const auto ptDeselector = [this](MovablePoint* const pt) {
        removePointFromSelection(pt);
    };
    point->setSelected(true, ptDeselector);
    mSelectedPoints_d.append(point);
    schedulePivotUpdate();
}

void Canvas::removePointFromSelection(MovablePoint * const point) {
    point->setSelected(false);
    mSelectedPoints_d.removeOne(point);

    schedulePivotUpdate();
}

void Canvas::removeSelectedPointsAndClearList() {
    if(mPressedPoint && mPressedPoint->isCtrlPoint()) {
        mPressedPoint->finishTransform();
        removePointFromSelection(mPressedPoint);
        mPressedPoint->remove();
        schedulePivotUpdate();
        return;
    }

    const auto selected = mSelectedPoints_d;
    for(const auto& point : selected) {
        point->setSelected(false);
        point->remove();
    }
    mSelectedPoints_d.clear(); schedulePivotUpdate();
}

void Canvas::clearPointsSelection() {
    for(const auto& point : mSelectedPoints_d) {
        if(point) point->setSelected(false);
    }

    mSelectedPoints_d.clear();
    if(mCurrentMode == CanvasMode::pointTransform) schedulePivotUpdate();
//    if(mLastPressedPoint) {
//        mLastPressedPoint->setSelected(false);
//        mLastPressedPoint = nullptr;
//    }
//    setCurrentEndPoint(nullptr);
}

void Canvas::clearLastPressedPoint() {
    if(mPressedPoint) {
        mPressedPoint->setSelected(false);
        mPressedPoint = nullptr;
    }
}

QPointF Canvas::getSelectedPointsAbsPivotPos() {
    if(mSelectedPoints_d.isEmpty()) return QPointF(0, 0);
    QPointF posSum(0, 0);
    for(const auto& point : mSelectedPoints_d) {
        posSum += point->getAbsolutePos();
    }
    const qreal invCount = 1./mSelectedPoints_d.count();
    return posSum*invCount;
}

bool Canvas::isPointSelectionEmpty() const {
    return mSelectedPoints_d.isEmpty();
}

int Canvas::getPointsSelectionCount() const {
    return mSelectedPoints_d.count();
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
