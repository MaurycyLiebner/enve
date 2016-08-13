#include "gradientpoint.h"
#include "vectorpath.h"

GradientPoint::GradientPoint(QPointF absPos, VectorPath *parent) :
    MovablePoint(absPos, parent, TYPE_GRADIENT_POINT)
{

}

void GradientPoint::setRelativePos(QPointF relPos, bool saveUndoRedo)
{
    MovablePoint::setRelativePos(relPos, saveUndoRedo);
    ((VectorPath*)mParent)->schedulePathUpdate();
}

void GradientPoint::setColor(QColor fillColor)
{
    mFillColor = fillColor;
    scheduleRepaint();
}

void GradientPoint::draw(QPainter *p)
{
    if(mHidden) {
        return;
    }
    p->setBrush(mFillColor);
    QPointF absPos = getAbsolutePos();
    p->drawRoundRect(QRectF(absPos - QPointF(mRadius, mRadius),
                            QSize(2*mRadius, 2*mRadius)) );
}
