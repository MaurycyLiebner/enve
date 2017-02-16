#include "gradientpoint.h"
#include "Boxes/vectorpath.h"

GradientPoint::GradientPoint(PathBox *parent) :
    MovablePoint(parent, TYPE_GRADIENT_POINT)
{
    getRelativePosAnimatorPtr()->setUpdater(new DisplayedFillStrokeSettingsUpdater(parent));
    mRelPos.setTraceKeyOnCurrentFrame(true);
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
    QPointF absPos = getAbsolutePos();
    p->drawRoundRect(QRectF(absPos - QPointF(mRadius, mRadius),
                            QSize(2*mRadius, 2*mRadius)) );

    if(mRelPos.isKeyOnCurrentFrame() ) {
        p->setBrush(Qt::red);
        p->setPen(QPen(Qt::black, 1.) );
        p->drawEllipse(absPos, 4, 4);
    }
}
