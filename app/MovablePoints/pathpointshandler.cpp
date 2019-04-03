#include "pathpointshandler.h"
#include "canvas.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "Animators/SmartPath/smartpathcollection.h"
#include "Animators/PathAnimators/smartpathcollectionhandler.h"
#include "PropertyUpdaters/smartnodepointupdater.h"

PathPointsHandler::PathPointsHandler(
        SmartPathCollectionHandler * const collectionHandler,
        SmartPathAnimator * const targetAnimator,
        BasicTransformAnimator * const parentTransform) :
    mCollectionHandler_k(collectionHandler),
    mTargetAnimator(targetAnimator),
    mParentTransform(parentTransform) {
    const auto updater = SPtrCreate(SmartNodePointUpdater)(this);
    mTargetAnimator->prp_setOwnUpdater(updater);
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
                                   const SkMatrix &totalTransform) const {
    Q_UNUSED(totalTransform);

    const bool keyOnCurrentFrame = mKeyOnCurrentFrame;
    if(currentCanvasMode == CanvasMode::MOVE_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            const auto& point = mPoints.at(i);
            point->drawNodePoint(canvas, currentCanvasMode, invScale,
                                 keyOnCurrentFrame);
        }
    } else if(currentCanvasMode == CanvasMode::ADD_SMART_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            const auto& point = mPoints.at(i);
            if(point->isEndPoint() || point->isSelected()) {
                point->drawNodePoint(canvas, currentCanvasMode, invScale,
                                     keyOnCurrentFrame);
            }
        }
    }
}

SmartNodePoint *PathPointsHandler::createNewNodePoint(const int &nodeId) {
    const auto newPt = SPtrCreate(SmartNodePoint)(
                this, mTargetAnimator, mParentTransform);
    mPoints.insert(nodeId, newPt);
    return newPt.get();
}

SmartNodePoint *PathPointsHandler::createAndAssignNewNodePoint(const int &nodeId) {
    const auto newPt = createNewNodePoint(nodeId);
    updatePoint(newPt, nodeId);
    return newPt;
}

SmartPath *PathPointsHandler::targetPath() const {
    return mTargetAnimator->getCurrentlyEditedPath();
}

void PathPointsHandler::updatePoint(const int &nodeId) {
    const auto& pt = mPoints.at(nodeId);
    updatePoint(pt.get(), nodeId);
}

void PathPointsHandler::updatePoint(SmartNodePoint * const pt,
                                    const int &nodeId) {
    pt->setNode(targetPath()->getNodePtr(nodeId));
}

void PathPointsHandler::updateAllPoints() {
    if(mBlockAllPointsUpdate) return;
    const int newCount = targetPath()->getNodeCount();
    while(newCount < mPoints.count()) mPoints.removeLast();
    while(mPoints.count() < newCount) createNewNodePoint(mPoints.count());
    for(int i = 0; i < mPoints.count(); i++) updatePoint(i);
}

void PathPointsHandler::setCtrlsMode(const int &nodeId,
                                     const CtrlsMode &mode) {
    blockAllPointsUpdate();
    mTargetAnimator->beforeBinaryPathChange();
    targetPath()->actionSetNormalNodeCtrlsMode(nodeId, mode);
    updatePoint(nodeId);
    mTargetAnimator->pathChanged();
    unblockAllPointsUpdate();
}

void PathPointsHandler::removeNode(const int &nodeId) {
    mTargetAnimator->actionRemoveNode(nodeId);
}

SmartNodePoint* PathPointsHandler::addFirstNode(const QPointF &relPos) {
    blockAllPointsUpdate();
    const int id = mTargetAnimator->actionAddFirstNode(relPos);
    unblockAllPointsUpdate();
    return createAndAssignNewNodePoint(id);
}

SmartNodePoint* PathPointsHandler::addNewAtEnd(const QPointF &relPos) {
    blockAllPointsUpdate();
    const int id = mTargetAnimator->actionAddNewAtEnd(relPos);
    unblockAllPointsUpdate();
    return createAndAssignNewNodePoint(id);
}

void PathPointsHandler::promoteToNormal(const int &nodeId) {
    blockAllPointsUpdate();
    mTargetAnimator->beforeBinaryPathChange();
    targetPath()->actionPromoteDissolvedNodeToNormal(nodeId);
    unblockAllPointsUpdate();
    mTargetAnimator->pathChanged();

    const int prevNormalId = targetPath()->prevNormalId(nodeId);
    const int nextNormalId = targetPath()->nextNormalId(nodeId);
    updatePoints(qMin(prevNormalId, nextNormalId),
                 qMax(prevNormalId, nextNormalId));
}

void PathPointsHandler::demoteToDissolved(const int &nodeId,
                                          const bool& approx) {
    blockAllPointsUpdate();
    mTargetAnimator->beforeBinaryPathChange();
    targetPath()->actionDemoteToDissolved(nodeId, approx);
    unblockAllPointsUpdate();
    mTargetAnimator->pathChanged();

    const int prevNormalId = targetPath()->prevNormalId(nodeId);
    const int nextNormalId = targetPath()->nextNormalId(nodeId);
    updatePoints(qMin(prevNormalId, nextNormalId),
                 qMax(prevNormalId, nextNormalId));
}

int PathPointsHandler::moveToClosestSegment(const int &nodeId,
                                            const QPointF &relPos) {
    NormalSegment::SubSegment minSubSeg{nullptr, nullptr, nullptr};
    qreal minDist = TEN_MIL;
    for(const auto& pt : mPoints) {
        const auto seg = pt->getNextNormalSegment();
        if(!seg.isValid()) continue;
        qreal dist;
        const auto subSeg = seg.getClosestSubSegmentForDummy(relPos, dist);
        if(dist < minDist) {
            minDist = dist;
            minSubSeg = subSeg;
        }
    }
    if(!minSubSeg.isValid()) return nodeId;
    const auto prevPt = minSubSeg.fFirstPt;
    const int prevNodeId = prevPt->getNodeId();
    if(prevNodeId == nodeId) return nodeId;
    const auto nextPt = minSubSeg.fLastPt;
    const int nextNodeId = nextPt->getNodeId();
    if(nextNodeId == nodeId) return nodeId;

    mTargetAnimator->actionMoveNodeBetween(nodeId, prevNodeId, nextNodeId);
    return (nodeId < prevNodeId ? prevNodeId : nextNodeId);
}

void PathPointsHandler::mergeNodes(const int &nodeId1, const int &nodeId2) {
    mTargetAnimator->actionMergeNodes(nodeId1, nodeId2);
}

SmartNodePoint* PathPointsHandler::divideSegment(const int &node1Id,
                                                 const int &node2Id,
                                                 const qreal &t) {
    const int id = mTargetAnimator->actionInsertNodeBetween(node1Id, node2Id, t);
    return mPoints.at(id).get();
}

void PathPointsHandler::createSegment(const int &node1Id, const int &node2Id) {
    mTargetAnimator->actionConnectNodes(node1Id, node2Id);
}

void PathPointsHandler::removeSegment(const NormalSegment &segment) {
    if(!segment.isValid()) return;
    const auto node1 = segment.getNodeAt(0);
    const auto node2 = segment.getNodeAt(1);
    if(!node1 || !node2) return;
    const int node1Id = node1->getNodeId();
    const int node2Id = node2->getNodeId();
    mTargetAnimator->actionDisconnectNodes(node1Id, node2Id);
    if(mTargetAnimator->hasDetached()) {
        const auto spColl = mTargetAnimator->getParent<SmartPathCollection>();
        if(!spColl) return;
        const auto newAnim = mTargetAnimator->createFromDetached();
        spColl->addPath(newAnim);
    }
}
