#include "gradientpoint.h"
#include "Boxes/vectorpath.h"

GradientPoint::GradientPoint(PathBox *parent) :
    MovablePoint(parent, TYPE_GRADIENT_POINT) {
    prp_setUpdater(new DisplayedFillStrokeSettingsUpdater(parent));
    anim_setTraceKeyOnCurrentFrame(true);
}

void GradientPoint::setRelativePos(QPointF relPos, bool saveUndoRedo)
{
    MovablePoint::setRelativePos(relPos, saveUndoRedo);
    ((VectorPath*)mParent)->updateDrawGradients();
}

void GradientPoint::moveByRel(QPointF relTranslatione)
{
    MovablePoint::moveByRel(relTranslatione);
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
    p->setPen(Qt::black);
    QPointF absPos = getAbsolutePos();
    QRectF rect = QRectF(absPos - QPointF(mRadius, mRadius),
                         QSize(2*mRadius, 2*mRadius));
    p->setPen(QPen(Qt::black, 1.5));
    p->drawEllipse(rect);
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(Qt::white, 0.75));
    p->drawEllipse(rect);

    if(prp_isKeyOnCurrentFrame() ) {
        p->setBrush(Qt::red);
        p->setPen(QPen(Qt::black, 1.) );
        p->drawEllipse(absPos, 4, 4);
    }
}
