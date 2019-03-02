#ifndef SMARTPATHCOLLECTIONHANDLER_H
#define SMARTPATHCOLLECTIONHANDLER_H

#include <smartPointers/selfref.h>
#include <smartPointers/stdselfref.h>
#include "smartPointers/stdpointer.h"

class BasicTransformAnimator;
class SmartPathCollection;
class PathPointsHandler;
class SmartPathAnimator;
class MovablePoint;
class NormalSegment;
enum CanvasMode : short;
#include "skia/skiaincludes.h"

class SmartPathCollectionHandler {
public:
    SmartPathCollectionHandler(
            BasicTransformAnimator * const parentTransform);

    PathPointsHandler* createNewPath();

    NormalSegment getNormalSegmentAtAbsPos(const QPointF &absPos,
                                           const qreal &canvasScaleInv);
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

    SkPath getPathAtRelFrame(const qreal &relFrame) const;
    SmartPathCollection* getAnimator() const {
        return mAnimator.get();
    }
private:
    const qsptr<SmartPathCollection> mAnimator;
    QList<stdsptr<PathPointsHandler>> mPointsHandlers;
    qptr<BasicTransformAnimator> mParentTransform;
};

#endif // SMARTPATHCOLLECTIONHANDLER_H
