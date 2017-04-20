#ifndef BOXPATHPOINT_H
#define BOXPATHPOINT_H
#include "movablepoint.h"
class BoundingBox;

class BoxPathPoint : public MovablePoint
{
public:
    BoxPathPoint(BoundingBox *box);
private:
    QPointF mSavedAbsPos;
};

#endif // BOXPATHPOINT_H
