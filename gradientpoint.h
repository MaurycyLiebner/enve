#ifndef GRADIENTPOINT_H
#define GRADIENTPOINT_H
#include "movablepoint.h"
#include "fillstrokesettings.h"
class VectorPath;

class GradientPoint : public MovablePoint
{
public:
    GradientPoint(PathBox *parent);
    void setRelativePos(const QPointF &relPos, const bool &saveUndoRedo);
    void moveByRel(QPointF relTranslatione);
    void setColor(QColor fillColor);
    void draw(QPainter *p);
private:
    QColor mFillColor = QColor(255, 0, 0, 155);
};

#endif // GRADIENTPOINT_H
