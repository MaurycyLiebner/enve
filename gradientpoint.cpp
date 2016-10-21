#include "gradientpoint.h"
#include "vectorpath.h"

GradientPoint::GradientPoint(PathBox *parent) :
    MovablePoint(parent, TYPE_GRADIENT_POINT)
{
    mRelPos.setTraceKeyOnCurrentFrame(true);
}

void GradientPoint::setRelativePos(QPointF relPos, bool saveUndoRedo)
{
    MovablePoint::setRelativePos(relPos, saveUndoRedo);
    ((VectorPath*)mParent)->updateDrawGradients();
}

void GradientPoint::moveBy(QPointF absTranslatione)
{
    MovablePoint::moveBy(absTranslatione);
    ((VectorPath*)mParent)->updateDrawGradients();
}

void GradientPoint::setColor(QColor fillColor)
{
    mFillColor = fillColor;
    
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

    if(mRelPos.isKeyOnCurrentFrame() ) {
        p->setBrush(Qt::red);
        p->setPen(QPen(Qt::black, 1.) );
        p->drawEllipse(absPos, 4, 4);
    }
}
