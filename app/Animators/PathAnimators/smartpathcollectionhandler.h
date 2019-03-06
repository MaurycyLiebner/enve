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
class SmartNodePoint;
class SmartVectorPath;
class Canvas;
enum CanvasMode : short;
#include "skia/skiaincludes.h"

class SmartPathCollectionHandler {
public:
    SmartPathCollectionHandler(
            BasicTransformAnimator * const parentTransform,
            SmartVectorPath* const parentPath);

    SmartNodePoint *createNewSubPathAtPos(const QPointF& pos);
    NormalSegment getNormalSegmentAtAbsPos(const QPointF &absPos,
                                           const qreal &canvasScaleInv);
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv) const;
    void selectAndAddContainedPointsToList(
            const QRectF &absRect,
            QList<stdptr<MovablePoint>> &list) const;
    void selectAllPoints(Canvas * const canvas);
    void drawPoints(SkCanvas * const canvas,
                    const CanvasMode &currentCanvasMode,
                    const SkScalar &invScale,
                    const SkMatrix &combinedTransform) const;

    SkPath getPathAtRelFrame(const qreal &relFrame) const;
    SmartPathCollection* getAnimator() const {
        return mAnimator.get();
    }
private:
    PathPointsHandler* createNewPath();

    const qsptr<SmartPathCollection> mAnimator;
    QList<stdsptr<PathPointsHandler>> mPointsHandlers;
    qptr<BasicTransformAnimator> mParentTransform;
    SmartVectorPath * const mParentPath;
};

#endif // SMARTPATHCOLLECTIONHANDLER_H
