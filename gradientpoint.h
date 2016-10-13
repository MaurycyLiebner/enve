#ifndef GRADIENTPOINT_H
#define GRADIENTPOINT_H
#include "movablepoint.h"
#include "fillstrokesettings.h"
class VectorPath;

class GradientPoint : public MovablePoint
{
public:
    GradientPoint(QPointF absPos, PathBox *parent);
    GradientPoint(int idT, PathBox *parent);
    void setRelativePos(QPointF relPos, bool saveUndoRedo);
    void moveBy(QPointF absTranslatione);
    void setColor(QColor fillColor);
    void draw(QPainter *p);
private:
    QColor mFillColor = QColor(255, 0, 0, 155);
};

#endif // GRADIENTPOINT_H
