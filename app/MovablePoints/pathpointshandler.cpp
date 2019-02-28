#include "pathpointshandler.h"
#include "canvas.h"
#include "Animators/SmartPath/smartpathanimator.h"

PathPointsHandler::PathPointsHandler(
        SmartPathAnimator * const targetAnimator,
        BasicTransformAnimator * const parentTransform) :
    mCurrentTarget(targetAnimator->getCurrentlyEditedPath()),
    mTargetAnimator(targetAnimator),
    mParentTransform(parentTransform) {
    updatePoints();
}

MovablePoint *PathPointsHandler::getPointAtAbsPos(
        const QPointF &absPtPos,
        const CanvasMode &currentCanvasMode,
        const qreal &canvasScaleInv) const {
    for(const auto& point : mPoints) {
        const auto pointToReturn = point->getPointAtAbsPos(
                    absPtPos, currentCanvasMode, canvasScaleInv);
        if(!pointToReturn) continue;
        return pointToReturn;
    }
    return nullptr;
}

void PathPointsHandler::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<stdptr<MovablePoint>> &list) const {
    for(const auto& point : mPoints) {
        point->rectPointsSelection(absRect, list);
    }
}

void PathPointsHandler::drawPoints(SkCanvas * const canvas,
                                   const CanvasMode &currentCanvasMode,
                                   const SkScalar &invScale,
                                   const SkMatrix &combinedTransform) const {
    Q_UNUSED(combinedTransform);

    const bool keyOnCurrentFrame = mKeyOnCurrentFrame;
    if(currentCanvasMode == CanvasMode::MOVE_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            auto point = mPoints.at(i);
            point->drawNodePoint(canvas, currentCanvasMode, invScale,
                                 keyOnCurrentFrame);
        }
    } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            auto point = mPoints.at(i);
            if(point->isEndPoint() || point->isSelected()) {
                point->drawNodePoint(canvas, currentCanvasMode, invScale,
                                     keyOnCurrentFrame);
            }
        }
    }
}

SmartNodePoint *PathPointsHandler::createNewNodePoint(const int &nodeId) {
    const auto newPt = SPtrCreate(SmartNodePoint)(nodeId, this,
                                                  mTargetAnimator,
                                                  mParentTransform);
    mPoints.insert(nodeId, newPt);
    return newPt.get();
}

void PathPointsHandler::updatePoint(const int &nodeId) {
    mPoints.at(nodeId)->updateFromNodeData();
}

void PathPointsHandler::updatePoints() {
    int j = 0;
    const int nodeCount = mCurrentTarget->getNodeCount();
    while(j < mPoints.count() && j < nodeCount) {
        updatePoint(j++);
    }
    while(j < nodeCount) {
        createNewNodePoint(j++);
    }
    while(j < mPoints.count()) {
        mPoints.removeAt(j++);
    }
}

void PathPointsHandler::setC0(const int &nodeId, const QPointF &c0) {
    mCurrentTarget->actionSetNormalNodeC0(nodeId, c0);
    updatePoint(nodeId);
}

void PathPointsHandler::setP1(const int &nodeId, const QPointF &p1) {
    mCurrentTarget->actionSetNormalNodeP1(nodeId, p1);
    updatePoint(nodeId);
}

void PathPointsHandler::setC2(const int &nodeId, const QPointF &c2) {
    mCurrentTarget->actionSetNormalNodeC2(nodeId, c2);
    updatePoint(nodeId);
}

void PathPointsHandler::setCtrlsMode(const int &nodeId,
                                     const CtrlsMode &mode) {
    mCurrentTarget->actionSetNormalNodeCtrlsMode(nodeId, mode);
    updatePoint(nodeId);
}

void PathPointsHandler::removeNode(const int &nodeId) {
    mCurrentTarget->actionRemoveNormalNode(nodeId);
    updatePoints();
}

void PathPointsHandler::addNewAtEnd(const int &nodeId,
                                    const NodePointValues &values) {
    mCurrentTarget->actionAppendNodeAtEndNode(nodeId, values);
    updatePoints();
}

void PathPointsHandler::setT(const int &nodeId, const qreal &t) {
    mCurrentTarget->actionSetDissolvedNodeT(nodeId, t);
    updatePoint(nodeId);
}

void PathPointsHandler::promoteToNormal(const int &nodeId) {
    mCurrentTarget->actionPromoteDissolvedNodeToNormal(nodeId);
    updatePoint(nodeId);
}

void PathPointsHandler::moveToClosestSegment(const int &nodeId,
                                             const QPointF &relPos) {
    NormalSegment::SubSegment minSubSeg{nullptr, nullptr, nullptr};
    qreal minDist = TEN_MIL;
    for(const auto& pt : mPoints) {
        const auto seg = pt->getNextEdge();
        qreal dist;
        const auto subSeg = seg.getClosestSubSegment(relPos, dist);
        if(dist < minDist) {
            minDist = dist;
            minSubSeg = subSeg;
        }
    }
    if(!minSubSeg.isValid()) return;
    const auto prevPt = minSubSeg.fFirstPt;
    const int prevNodeId = prevPt->getNodeId();
    if(prevNodeId == nodeId) return;
    const auto nextPt = minSubSeg.fLastPt;
    const int nextNodeId = nextPt->getNodeId();
    if(nextNodeId == nodeId) return;
    mCurrentTarget->actionMoveNodeBetween(nodeId, prevNodeId, nextNodeId);
    updatePoint(nodeId);
    updatePoint(prevNodeId);
    updatePoint(nextNodeId);
}

void PathPointsHandler::divideSegment(const int &node1Id,
                                      const int &node2Id,
                                      const qreal &t) {
    mCurrentTarget->actionInsertNodeBetween(node1Id, node2Id, t);
}

void PathPointsHandler::createSegment(const int &node1Id,
                                      const int &node2Id) {
    mCurrentTarget->actionConnectNodes(node1Id, node2Id);
    updatePoint(node1Id);
    updatePoint(node2Id);
}

void PathPointsHandler::removeSegment(const NormalSegment &segment) {
    if(!segment.isValid()) return;
    const auto node1 = segment.getNodeAt(0);
    const auto node2 = segment.getNodeAt(1);
    if(!node1 || !node2) return;
    const int node1Id = node1->getNodeId();
    const int node2Id = node2->getNodeId();
    mCurrentTarget->actionDisconnectNodes(node1Id, node2Id);
    updatePoint(node1Id);
    updatePoint(node2Id);
}
