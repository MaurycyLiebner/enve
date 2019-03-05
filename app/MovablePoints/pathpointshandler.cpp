#include "pathpointshandler.h"
#include "canvas.h"
#include "Animators/SmartPath/smartpathanimator.h"

PathPointsHandler::PathPointsHandler(
        SmartPathAnimator * const targetAnimator,
        BasicTransformAnimator * const parentTransform) :
    mCurrentTarget(targetAnimator->getCurrentlyEditedPath()),
    mTargetAnimator(targetAnimator),
    mParentTransform(parentTransform) {
    QObject::connect(mTargetAnimator,
                     &SmartPathAnimator::pathChangedAfterFrameChange,
                     [this]() { this->updatePoints(); });
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

void PathPointsHandler::selectAllPoints(Canvas * const canvas) {
    for(const auto& point : mPoints) {
        canvas->addPointToSelection(point.get());
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
    } else if(currentCanvasMode == CanvasMode::ADD_SMART_POINT) {
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

void PathPointsHandler::updateNextSegmentDnDForPoint(const int &nodeId) {
    mPoints.at(nodeId)->updateNextSegmentDnD();
}

void PathPointsHandler::updatePoint(const int &nodeId) {
    mPoints.at(nodeId)->updateNode();
}

void PathPointsHandler::updatePoints() {
    const int oldCount = mPoints.count();
    const int newCount = mCurrentTarget->getNodeCount();
    for(int i = oldCount; i < newCount; i++) {
        createNewNodePoint(i);
    }
    for(int i = 0; i < oldCount; i++) {
        updatePoint(i);
    }
    for(int i = newCount; i < oldCount; i++) {
        mPoints.removeAt(i);
    }
}

void PathPointsHandler::setCtrlsMode(const int &nodeId,
                                     const CtrlsMode &mode) {
    mCurrentTarget->actionSetNormalNodeCtrlsMode(nodeId, mode);
    updatePoint(nodeId);
    mTargetAnimator->pathChanged();
}

void PathPointsHandler::removeNode(const int &nodeId) {
    mCurrentTarget->actionRemoveNormalNode(nodeId);
    updatePoints();
    mTargetAnimator->pathChanged();
}

SmartNodePoint* PathPointsHandler::addFirstNode(const QPointF &relPos) {
    const int id = mCurrentTarget->actionAddFirstNode(relPos, relPos, relPos);
    mTargetAnimator->pathChanged();
    return createNewNodePoint(id);
}

SmartNodePoint* PathPointsHandler::addNewAtEnd(const int &nodeId,
                                               const QPointF &relPos) {
    const int id = mCurrentTarget->actionAppendNodeAtEndNode(
                nodeId, {relPos, relPos, relPos});
    mTargetAnimator->pathChanged();
    return createNewNodePoint(id);
}

void PathPointsHandler::promoteToNormal(const int &nodeId) {
    mCurrentTarget->actionPromoteDissolvedNodeToNormal(nodeId);
    updatePoint(nodeId);
}

bool PathPointsHandler::moveToClosestSegment(const int &nodeId,
                                             const QPointF &relPos) {
    NormalSegment::SubSegment minSubSeg{nullptr, nullptr, nullptr};
    qreal minDist = TEN_MIL;
    for(const auto& pt : mPoints) {
        const auto seg = pt->getNextNormalSegment();
        qreal dist;
        const auto subSeg = seg.getClosestSubSegmentForDummy(relPos, dist);
        if(dist < minDist) {
            minDist = dist;
            minSubSeg = subSeg;
        }
    }
    qDebug() << "";
    qDebug() << minDist;
    if(!minSubSeg.isValid()) return false;
    qDebug() << "VALID";
    const auto prevPt = minSubSeg.fFirstPt;
    const int prevNodeId = prevPt->getNodeId();
    if(prevNodeId == nodeId) return false;
    const auto nextPt = minSubSeg.fLastPt;
    const int nextNodeId = nextPt->getNodeId();
    if(nextNodeId == nodeId) return false;
    const Node * const node = mCurrentTarget->getNodePtr(nodeId);
    const int oldPrevNodeId = node->getPrevNodeId();
    const int oldNextNodeId = node->getNextNodeId();
    mCurrentTarget->actionMoveNodeBetween(nodeId, prevNodeId, nextNodeId);
    updatePoint(nodeId);
    updatePoint(prevNodeId);
    updatePoint(nextNodeId);
    if(oldPrevNodeId != -1) updatePoint(oldPrevNodeId);
    if(oldNextNodeId != -1) updatePoint(oldNextNodeId);
    if(oldPrevNodeId != -1) updateNextSegmentDnDForPoint(oldPrevNodeId);
    updateNextSegmentDnDForPoint(prevNodeId);
    return true;
}

void PathPointsHandler::divideSegment(const int &node1Id,
                                      const int &node2Id,
                                      const qreal &t) {
    mCurrentTarget->actionInsertNodeBetween(node1Id, node2Id, t);
    updatePoints();
    mTargetAnimator->pathChanged();
}

void PathPointsHandler::createSegment(const int &node1Id,
                                      const int &node2Id) {
    mCurrentTarget->actionConnectNodes(node1Id, node2Id);
    updatePoints();
    mTargetAnimator->pathChanged();
}

void PathPointsHandler::removeSegment(const NormalSegment &segment) {
    if(!segment.isValid()) return;
    const auto node1 = segment.getNodeAt(0);
    const auto node2 = segment.getNodeAt(1);
    if(!node1 || !node2) return;
    const int node1Id = node1->getNodeId();
    const int node2Id = node2->getNodeId();
    mCurrentTarget->actionDisconnectNodes(node1Id, node2Id);
    updatePoints();
    mTargetAnimator->pathChanged();
}
