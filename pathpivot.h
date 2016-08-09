#ifndef PATHPIVOT_H
#define PATHPIVOT_H
#include "movablepoint.h"

class PathPivot : public MovablePoint
{
public:
    PathPivot(VectorPath *path);

    void draw(QPainter *p);

    bool wasChanged();
private:
    bool mWasChanged = false;
};

#endif // PATHPIVOT_H
