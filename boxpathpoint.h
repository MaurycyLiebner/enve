#ifndef BOXPATHPOINT_H
#define BOXPATHPOINT_H
#include "movablepoint.h"
class BoundingBox;

class BoxPathPoint : public MovablePoint
{
public:
    BoxPathPoint(BoundingBox *box);

    void startTransform();
    void finishTransform();
    void moveByAbs(const QPointF &absTranslatione);
    void drawSk(SkCanvas *canvas, const SkScalar &invScale);
private:
    QPointF mSavedAbsPos;
};

#endif // BOXPATHPOINT_H
