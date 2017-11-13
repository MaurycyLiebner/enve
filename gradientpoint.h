#ifndef GRADIENTPOINT_H
#define GRADIENTPOINT_H
#include "pointanimator.h"
class PathBox;

class GradientPoint : public PointAnimator {
public:
    GradientPoint(PathBox *parent);
    void setRelativePos(const QPointF &relPos);
    void moveByRel(const QPointF &relTranslatione);
    void setColor(const QColor &fillColor);
    void drawSk(SkCanvas *canvas,
                const SkScalar &invScale);
private:
    QColor mFillColor = QColor(255, 0, 0, 155);
};

#endif // GRADIENTPOINT_H
