#ifndef GRADIENTPOINT_H
#define GRADIENTPOINT_H
#include "animatedpoint.h"
class PathBox;

class GradientPoint : public AnimatedPoint {
    friend class StdSelfRef;
public:
    void setRelativePos(const QPointF &relPos);
    void moveByRel(const QPointF &relTranslatione);
    void setColor(const QColor &fillColor);
    void drawSk(SkCanvas * const canvas,
                const SkScalar &invScale);
protected:
    GradientPoint(QPointFAnimator* const associatedAnimator,
                  PathBox * const parent);
private:
    QColor mFillColor = QColor(255, 0, 0, 155);
};

#endif // GRADIENTPOINT_H
