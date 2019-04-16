#ifndef BOXPATHPOINT_H
#define BOXPATHPOINT_H
#include "animatedpoint.h"
class BoxTransformAnimator;
class BoxPathPoint : public AnimatedPoint {
    friend class StdSelfRef;
protected:
    BoxPathPoint(QPointFAnimator * const associatedAnimator,
                 BoxTransformAnimator * const boxTrans);
public:
    void setRelativePos(const QPointF& relPos);

    void startTransform();
    void finishTransform();

    void drawSk(SkCanvas * const canvas, const SkScalar &invScale);
private:
    BoxTransformAnimator * const mBoxTransform_cv;
};

#endif // BOXPATHPOINT_H
