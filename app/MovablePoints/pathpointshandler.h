#ifndef PATHPOINTSHANDLER_H
#define PATHPOINTSHANDLER_H
#include "Animators/SmartPath/smartpathcontainer.h"
#include "smartnodepoint.h"

class PathPointsHandler : public StdSelfRef {
    friend class StdSelfRef;
public:
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv) const;
    void selectAndAddContainedPointsToList(
            const QRectF &absRect,
            QList<stdptr<MovablePoint>> &list) const;
    void drawPoints(SkCanvas * const canvas,
                    const CanvasMode &currentCanvasMode,
                    const SkScalar &invScale,
                    const SkMatrix &combinedTransform) const;

    SmartNodePoint* getPointWithId(const int& id) const {
        return mPoints.at(id).get();
    }

    // actions on NORMAL
    void setCtrlsMode(const int& nodeId, const CtrlsMode& mode);
    void removeNode(const int& nodeId);
    void addNewAtEnd(const int& nodeId, const QPointF &relPos);

    // actions on DISSOLVED

    // actions on DUMMY and DISSOLVED
    void promoteToNormal(const int& nodeId);
    void moveToClosestSegment(const int& nodeId, const QPointF& relPos);

    // actions on normal-normal segments
    void divideSegment(const int& node1Id, const int& node2Id,
                       const qreal& t);
    void createSegment(const int& node1Id, const int& node2Id);
    void removeSegment(const NormalSegment &segment);

    SmartNodePoint* createNewNodePoint(const int& nodeId);

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
    void updatePoint(const int& nodeId);
    void updatePoints();

    QList<stdsptr<SmartNodePoint>> mPoints;
    SmartPath* mCurrentTarget;
    SmartPathAnimator * const mTargetAnimator;
    BasicTransformAnimator * const mParentTransform;
    bool mKeyOnCurrentFrame = false;
};

#endif // PATHPOINTSHANDLER_H
