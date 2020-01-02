// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#ifndef PATHPOINTSHANDLER_H
#define PATHPOINTSHANDLER_H
#include "Animators/SmartPath/smartpath.h"
#include "smartnodepoint.h"
#include "pointshandler.h"
#include "simpletask.h"
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

    void scheduleRemoveNode(const int nodeId) {
        mRemoveNodes << nodeId;
        scheduleNodesRemoval();
    }

    // actions on NORMAL
    void setCtrlsMode(const int nodeId, const CtrlsMode mode);
    void removeNode(const int nodeId, const bool approx);
    SmartNodePoint *addNewAtStart(const QPointF &relPos);
    SmartNodePoint *addNewAtEnd(const QPointF &relPos);
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
    SimpleTaskScheduler scheduleNodesRemoval;

    void flushNodesRemoval() {
        auto nodes = mRemoveNodes;
        mRemoveNodes.clear();
        std::sort(nodes.begin(), nodes.end());
        for(auto it = nodes.rbegin(); it != nodes.rend(); it++) {
            removeNode(*it, false);
        }
    }

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

    SmartPathAnimator * const mTargetAnimator;
    bool mKeyOnCurrentFrame = false;

    QList<int> mRemoveNodes;
};

#endif // PATHPOINTSHANDLER_H
