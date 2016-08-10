#include "pathpivot.h"

PathPivot::PathPivot(BoundingBox *parent) :
    MovablePoint(QPointF(0.f, 0.f), parent,
                 MovablePointType::TYPE_PIVOT_POINT, 15.f)
{

}

void PathPivot::draw(QPainter *p)
{
    if(mHidden) {
        return;
    }
    if(mSelected) {
        p->setBrush(QColor(0, 255, 0, 155));
    } else {
        p->setBrush(QColor(0, 255, 0, 75));
    }
    p->drawEllipse(getAbsolutePos(),
                   mRadius, mRadius);
    p->save();
    p->translate(getAbsolutePos());
    qreal halfRadius = mRadius*0.5f;
    p->drawLine(QPointF(-halfRadius, 0), QPointF(halfRadius, 0));
    p->drawLine(QPointF(0, -halfRadius), QPointF(0, halfRadius));
    p->restore();
}
