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

SmartPath *PathPointsHandler::targetPath() const {
    return mTargetAnimator->getCurrentlyEditedPath();
}

void PathPointsHandler::updateNextSegmentDnDForPoint(const int &nodeId) {
    mPoints.at(nodeId)->updateNextSegmentDnD();
}

void PathPointsHandler::updatePoint(const int &nodeId) {
    const auto pt = mPoints.at(nodeId);
    pt->setNodeId(nodeId);
    pt->afterAllNodesUpdated();
}

void PathPointsHandler::updateAllPoints() {
    if(mBlockAllPointsUpdate) return;
    const int newCount = targetPath()->getNodeCount();
    while(newCount < mPoints.count()) {
        mPoints.removeLast();
    }
    for(int i = 0; i < mPoints.count(); i++) {
        mPoints.at(i)->setOutdated();
    }
    for(int i = 0; i < mPoints.count(); i++) {
        mPoints.at(i)->setNodeId(i);
    }
    while(mPoints.count() < newCount) {
        createNewNodePoint(mPoints.count());
    }
    for(const auto& pt : mPoints) {
        pt->afterAllNodesUpdated();
    }
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
    return createNewNodePoint(id);
}

SmartNodePoint* PathPointsHandler::addNewAtEnd(const int &nodeId,
                                               const QPointF &relPos) {
    blockAllPointsUpdate();
    const int id = mTargetAnimator->actionAddNewAtEnd(nodeId, relPos);
    unblockAllPointsUpdate();
    return createNewNodePoint(id);
}

void PathPointsHandler::promoteToNormal(const int &nodeId) {
    blockAllPointsUpdate();
    mTargetAnimator->beforeBinaryPathChange();
    targetPath()->actionPromoteDissolvedNodeToNormal(nodeId);
    unblockAllPointsUpdate();
    updatePoint(nodeId);
}

bool PathPointsHandler::moveToClosestSegment(const int &nodeId,
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
    if(!minSubSeg.isValid()) return false;
    const auto prevPt = minSubSeg.fFirstPt;
    const int prevNodeId = prevPt->getNodeId();
    if(prevNodeId == nodeId) return false;
    const auto nextPt = minSubSeg.fLastPt;
    const int nextNodeId = nextPt->getNodeId();
    if(nextNodeId == nodeId) return false;

    const int oldPrevNodeId = targetPath()->prevNodeId(nodeId);
    const int oldNextNodeId = targetPath()->nextNodeId(nodeId);
    blockAllPointsUpdate();
    mTargetAnimator->beforeBinaryPathChange();
    targetPath()->actionMoveNodeBetween(nodeId, prevNodeId, nextNodeId);
    updatePoint(nodeId);
    updatePoint(prevNodeId);
    updatePoint(nextNodeId);
    if(oldPrevNodeId != -1) updatePoint(oldPrevNodeId);
    if(oldNextNodeId != -1) updatePoint(oldNextNodeId);
    if(oldPrevNodeId != -1) updateNextSegmentDnDForPoint(oldPrevNodeId);
    updateNextSegmentDnDForPoint(prevNodeId);
    unblockAllPointsUpdate();
    return true;
}

void PathPointsHandler::divideSegment(const int &node1Id,
                                      const int &node2Id,
                                      const qreal &t) {
    mTargetAnimator->actionInsertNodeBetween(node1Id, node2Id, t);
}

void PathPointsHandler::createSegment(const int &node1Id,
                                      const int &node2Id) {
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
        spColl->ca_addChildAnimator(newAnim);
        mCollectionHandler_k->createHandlerForAnimator(newAnim.get());
    }
}
