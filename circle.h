#ifndef CIRCLE_H
#define CIRCLE_H
#include "boundingbox.h"

class Circle : public BoundingBox
{
public:
    Circle(BoxesGroup *parent);

    void draw(QPainter *p);

    void setVerticalRadius(qreal );
    void setHorizontalRadius(qreal horizontalRadius);
    void setRadius(qreal radius);
private:
    MovablePoint *mCenter;
    MovablePoint *mHorizontalRadius;
    MovablePoint *mVerticalRadius;
};

#endif // CIRCLE_H
