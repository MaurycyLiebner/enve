#ifndef PATHPOINTSHANDLER_H
#define PATHPOINTSHANDLER_H
#include "Animators/SmartPath/smartpathcontainer.h"
#include "smartnodepoint.h"
class Canvas;

class PathPointsHandler : public StdSelfRef {
    friend class StdSelfRef;
public:
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv) const;
    
    NormalSegment getNormalSegmentAtAbsPos(const QPointF &absPos,
                                           const qreal &canvasScaleInv) const {
        qreal minDist = 5*canvasScaleInv;
        NormalSegment bestSeg;
        for(const auto& point : mPoints) {
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

    void selectAndAddContainedPointsToList(
            const QRectF &absRect,
            QList<stdptr<MovablePoint>> &list) const;
    void selectAllPoints(Canvas * const canvas);
    void drawPoints(SkCanvas * const canvas,
                    const CanvasMode &currentCanvasMode,
                    const SkScalar &invScale,
                    const SkMatrix &combinedTransform) const;

    SmartNodePoint* getPointWithId(const int& id) const {
        if(id < 0) return nullptr;
        if(id >= mPoints.count()) return nullptr;
        return mPoints.at(id).get();
    }

    // actions on NORMAL
    void setCtrlsMode(const int& nodeId, const CtrlsMode& mode);
    void removeNode(const int& nodeId);
    SmartNodePoint *addNewAtEnd(const int& nodeId, const QPointF &relPos);
    SmartNodePoint *addFirstNode(const QPointF &relPos);
    // actions on DISSOLVED

    // actions on DUMMY and DISSOLVED
    void promoteToNormal(const int& nodeId);
    bool moveToClosestSegment(const int& nodeId, const QPointF& relPos);

    // actions on normal-normal segments
    void divideSegment(const int& node1Id, const int& node2Id,
                       const qreal& t);
    void createSegment(const int& node1Id, const int& node2Id);
    void removeSegment(const NormalSegment &segment);


    SmartNodePoint* getPrevNormalNode(const int& startId) const {
        return getPointWithId(mCurrentTarget->prevNormalId(startId));
    }

    SmartNodePoint* getNextNormalNode(const int& startId) const {
        return getPointWithId(mCurrentTarget->nextNormalId(startId));
    }
protected:
    PathPointsHandler(SmartPathAnimator * const targetAnimator,
                      BasicTransformAnimator * const parentTransform);

    void setCurrentlyEditedPath(SmartPath * const currentTarget) {
        if(currentTarget == mCurrentTarget) return;
        mCurrentTarget = currentTarget;
        updatePoints();
    }
private:
    void updateNextSegmentDnDForPoint(const int& nodeId);
    void updatePoint(const int& nodeId);
    void updatePoints();
    SmartNodePoint* createNewNodePoint(const int& nodeId);

    QList<stdsptr<SmartNodePoint>> mPoints;
    SmartPath* mCurrentTarget;
    SmartPathAnimator * const mTargetAnimator;
    BasicTransformAnimator * const mParentTransform;
    bool mKeyOnCurrentFrame = false;
};

#endif // PATHPOINTSHANDLER_H
