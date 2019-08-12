#ifndef GRADIENTPOINT_H
#define GRADIENTPOINT_H
#include "MovablePoints/animatedpoint.h"
class PathBox;

class GradientPoint : public AnimatedPoint {
    e_OBJECT
public:
    GradientPoint(QPointFAnimator* const associatedAnimator,
                  PathBox * const parent);
    void setColor(const QColor &fillColor);
    void drawSk(SkCanvas * const canvas,
                const CanvasMode mode,
                const float invScale,
                const bool keyOnCurrent,
                const bool ctrlPressed);
private:
    QColor mFillColor;
};

#endif // GRADIENTPOINT_H
