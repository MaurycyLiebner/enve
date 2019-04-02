#ifndef BOXPATHPOINT_H
#define BOXPATHPOINT_H
#include "animatedpoint.h"
class BoundingBox;
class BoxTransformAnimator;
class BoxPathPoint : public AnimatedPoint {
    friend class StdSelfRef;
protected:
    BoxPathPoint(QPointFAnimator * const associatedAnimator,
                 BoxTransformAnimator * const box);
public:
    void startTransform();
    void finishTransform();
    void moveByAbs(const QPointF &absTrans);
    void drawSk(SkCanvas * const canvas, const SkScalar &invScale);
private:
    QPointF mSavedAbsPos;
    BoxTransformAnimator * mBoxTransform_cv;
};

#endif // BOXPATHPOINT_H
