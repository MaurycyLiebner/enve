#ifndef PATHPIVOT_H
#define PATHPIVOT_H
#include "movablepoint.h"

class PathPivot : public MovablePoint
{
public:
    PathPivot(BoundingBox *parent);

    void draw(QPainter *p);
};

#endif // PATHPIVOT_H
