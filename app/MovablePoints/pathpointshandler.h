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
    void setC0(const int& nodeId, const QPointF& c0);
    void setP1(const int& nodeId, const QPointF& p1);
    void setC2(const int& nodeId, const QPointF& c2);
    void setCtrlsMode(const int& nodeId, const CtrlsMode& mode);
    void removeNode(const int& nodeId);
    void addNewAtEnd(const int& nodeId, const NodePointValues& values);

    // actions on DISSOLVED
    void setT(const int& nodeId, const qreal& t);

    // actions on DUMMY and DISSOLVED
    void promoteToNormal(const int& nodeId);
    void moveToDifferentSegment(const int& nodeId, const QPointF& pos);

    // actions on normal-normal segments
    void dragSegment(const int& node1Id, const int& node2Id,
                     const qreal& t);
    void divideSegment(const int& node1Id, const int& node2Id,
                       const qreal& t);
    void createSegment(const int& node1Id, const int& node2Id);
    void removeSegment(const int& node1Id, const int& node2Id);

    SmartNodePoint* createNewNodePoint(const int& nodeId);
protected:
    PathPointsHandler(SmartPathAnimator * const targetAnimator,
                      BasicTransformAnimator * const parentTransform);

    void setCurrentlyEditedPath(PathBase * const currentTarget) {
        if(currentTarget == mCurrentTarget) return;
        mCurrentTarget = currentTarget;
        updatePoints();
    }
private:
    void updatePoint(const int& nodeId);
    void updatePoints();

    QList<stdsptr<SmartNodePoint>> mPoints;
    PathBase* mCurrentTarget;
    SmartPathAnimator * const mTargetAnimator;
    BasicTransformAnimator * const mParentTransform;
    bool mKeyOnCurrentFrame = false;
};

#endif // PATHPOINTSHANDLER_H
