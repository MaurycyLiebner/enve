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
    friend class PathPointsHandler;
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
                    const SkMatrix &totalTransform) const;

    SkPath getPathAtRelFrame(const qreal &relFrame) const;
    SmartPathCollection* getAnimator() const {
        return mAnimator.get();
    }

    qsptr<SmartPathAnimator> takeAnimatorAt(const int& id);
    PathPointsHandler* addAnimator(const qsptr<SmartPathAnimator>& anim);

    void moveAllFrom(SmartPathCollectionHandler * const from);

    void applyTransform(const QMatrix& transform);
    int numberOfAnimators() const;
protected:
    PathPointsHandler *createHandlerForAnimator(
            SmartPathAnimator * const newAnimator);
private:
    PathPointsHandler* createNewPath();

    const qsptr<SmartPathCollection> mAnimator;
    QList<stdsptr<PathPointsHandler>> mPointsHandlers;
    qptr<BasicTransformAnimator> mParentTransform;
    SmartVectorPath * const mParentPath;
};

#endif // SMARTPATHCOLLECTIONHANDLER_H
