#ifndef PATHPOINTSHANDLER_H
#define PATHPOINTSHANDLER_H
#include "Animators/SmartPath/smartpath.h"
#include "smartnodepoint.h"
#include "pointshandler.h"
class Canvas;
class SmartPathCollectionHandler;

class PathPointsHandler : public PointsHandler {
    e_OBJECT
protected:
    PathPointsHandler(SmartPathAnimator * const targetAnimator);
public:
    NormalSegment getNormalSegment(const QPointF &absPos,
                                   const qreal invScale) const {
        qreal minDist = 5*invScale;
        NormalSegment bestSeg;
        for(int i = 0; i < count(); i++) {
            const auto point = getPointWithId<SmartNodePoint>(i);
            const auto nSeg = point->getNextNormalSegment();
            if(!nSeg.isValid()) continue;
            auto absSeg = nSeg.getAsAbsSegment();
            const qreal dist = absSeg.minDistanceTo(absPos);
            if(dist < minDist) {
                minDist = dist;
                bestSeg = nSeg;
            }
        }
        return bestSeg;
    }

    // actions on NORMAL
    void setCtrlsMode(const int nodeId, const CtrlsMode mode);
    void removeNode(const int nodeId, const bool approx);
    SmartNodePoint *addNewAtEnd(const QPointF &relPos);
    SmartNodePoint *addFirstNode(const QPointF &relPos);
    // actions on DISSOLVED

    // actions on DUMMY and DISSOLVED
    void promoteToNormal(const int nodeId);
    void demoteToDissolved(const int nodeId, const bool approx);
    int moveToClosestSegment(const int nodeId, const QPointF& relPos);

    void mergeNodes(const int nodeId1, const int nodeId2);

    // actions on normal-normal segments
    SmartNodePoint *divideSegment(const int node1Id, const int node2Id,
                                  const qreal t);
    void createSegment(const int node1Id, const int node2Id);
    void removeSegment(const NormalSegment &segment);

    int getPrevNodeId(const int startId) const {
        return targetPath()->prevNodeId(startId);
    }

    int getNextNodeId(const int startId) const {
        return targetPath()->nextNodeId(startId);
    }

    SmartNodePoint* getPrevNode(const int startId) const {
        return getPointWithId<SmartNodePoint>(getPrevNodeId(startId));
    }

    SmartNodePoint* getNextNode(const int startId) const {
        return getPointWithId<SmartNodePoint>(getNextNodeId(startId));
    }

    SmartNodePoint* getPrevNormalNode(const int startId) const {
        return getPointWithId<SmartNodePoint>(targetPath()->prevNormalId(startId));
    }

    SmartNodePoint* getNextNormalNode(const int startId) const {
        const int normalId = targetPath()->nextNormalId(startId);
        return getPointWithId<SmartNodePoint>(normalId);
    }

    SmartNodePoint* getClosestNode(const QPointF& absPos, const qreal& maxDist) const;

    void updateAllPoints();

    const SmartPathAnimator * getAnimator() {
        return mTargetAnimator;
    }
private:
    void updatePoints(int min, int max) {
        const int lastId = count() - 1;
        min = clamp(min, 0, lastId);
        max = clamp(max, 0, lastId);
        for(int i = min; i <= max; i++) updatePoint(i);
    }

    void updatePoint(const int nodeId);
    void updatePoint(SmartNodePoint * const pt, const int nodeId);

    SmartNodePoint* createNewNodePoint(const int nodeId);
    SmartNodePoint* createAndAssignNewNodePoint(const int nodeId);
    SmartPath* targetPath() const;
    void blockAllPointsUpdate() {
        mBlockAllPointsUpdate = true;
    }
    void unblockAllPointsUpdate() {
        mBlockAllPointsUpdate = false;
    }

    SmartPathAnimator * const mTargetAnimator;
    bool mKeyOnCurrentFrame = false;
    bool mBlockAllPointsUpdate = false;
};

#endif // PATHPOINTSHANDLER_H
