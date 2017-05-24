#include "gradientpoint.h"
#include "Boxes/vectorpath.h"
#include "Animators/animatorupdater.h"
#include "pointhelpers.h"

GradientPoint::GradientPoint(PathBox *parent) :
    MovablePoint(parent, TYPE_GRADIENT_POINT) {
    prp_setUpdater(new DisplayedFillStrokeSettingsUpdater(parent));
    anim_setTraceKeyOnCurrentFrame(true);
}

void GradientPoint::setRelativePos(const QPointF &relPos, const bool &saveUndoRedo)
{
    MovablePoint::setRelativePos(relPos, saveUndoRedo);
    ((VectorPath*)mParent)->updateDrawGradients();
}

void GradientPoint::moveByRel(const QPointF &relTranslatione)
{
    MovablePoint::moveByRel(relTranslatione);
    ((VectorPath*)mParent)->updateDrawGradients();
}

void GradientPoint::setColor(QColor fillColor)
{
    mFillColor = fillColor;
    
}

void GradientPoint::draw(QPainter *p) {
    if(mHidden) {
        return;
    }
    p->save();
    p->setBrush(mFillColor);
    QPointF absPos = getAbsolutePos();

    QPen pen = p->pen();
    pen.setColor(Qt::black);
    pen.setWidthF(1.5);
    p->setPen(pen);
    drawCosmeticEllipse(p, absPos,
                        mRadius, mRadius);
    p->setBrush(Qt::NoBrush);
    pen.setColor(Qt::white);
    pen.setWidthF(0.75);
    p->setPen(pen);
    drawCosmeticEllipse(p, absPos,
                        mRadius, mRadius);

    if(prp_isKeyOnCurrentFrame() ) {
        p->setBrush(Qt::red);
        pen.setColor(Qt::black);
        pen.setWidthF(1.);
        p->setPen(pen);
        drawCosmeticEllipse(p, absPos, 3, 3);
    }
    p->restore();
}
