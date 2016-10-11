#include "circle.h"

#include "movablepoint.h"

Circle::Circle(BoxesGroup *parent) : BoundingBox(parent, TYPE_CIRCLE)
{
    mCenter = new MovablePoint(0., 0., this, TYPE_PATH_POINT);
    mHorizontalRadius = new MovablePoint(10., 0., this, TYPE_PATH_POINT);
    mVerticalRadius = new MovablePoint(0., 10., this, TYPE_PATH_POINT);
}

void Circle::draw(QPainter *p)
{
    if(mVisible) {
        p->setBrush(Qt::red);
        p->setPen(QPen(Qt::black, 1.));
        QPointF centerPos = mCenter->getRelativePos();
        QPainterPath path;
        path.addEllipse(centerPos,
                        (centerPos - mHorizontalRadius->getRelativePos()).x(),
                        (centerPos - mVerticalRadius->getRelativePos()).y() );
        path = mCombinedTransformMatrix.map(path);
        p->drawPath(path);
    }
}
