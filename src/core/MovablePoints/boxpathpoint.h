#ifndef BOXPATHPOINT_H
#define BOXPATHPOINT_H
#include "animatedpoint.h"
class BoxTransformAnimator;
class BoxPathPoint : public AnimatedPoint {
    e_OBJECT
protected:
    BoxPathPoint(QPointFAnimator * const associatedAnimator,
                 BoxTransformAnimator * const boxTrans);
public:
    void setRelativePos(const QPointF& relPos);

    void startTransform();
    void finishTransform();

    bool isVisible(const CanvasMode mode) const {
        return mode == CanvasMode::boxTransform;
    }

    void drawSk(SkCanvas * const canvas,
                const CanvasMode mode,
                const float invScale,
                const bool keyOnCurrent,
                const bool ctrlPressed);
};

#endif // BOXPATHPOINT_H
