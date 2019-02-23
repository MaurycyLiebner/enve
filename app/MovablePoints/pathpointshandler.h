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
    SmartNodePoint* createNewNodePoint(const int& nodeId);

    QList<stdsptr<SmartNodePoint>> mPoints;
    PathBase* mCurrentTarget;
    SmartPathAnimator * const mTargetAnimator;
    BasicTransformAnimator * const mParentTransform;
    bool mKeyOnCurrentFrame = false;
};

#endif // PATHPOINTSHANDLER_H
