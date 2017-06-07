#include "gradientpoint.h"
#include "Boxes/vectorpath.h"
#include "Animators/animatorupdater.h"
#include "pointhelpers.h"

GradientPoint::GradientPoint(PathBox *parent) :
    MovablePoint(parent, TYPE_GRADIENT_POINT) {
    prp_setUpdater(new DisplayedFillStrokeSettingsUpdater(parent));
    anim_setTraceKeyOnCurrentFrame(true);
}

void GradientPoint::setRelativePos(const QPointF &relPos,
                                   const bool &saveUndoRedo) {
    MovablePoint::setRelativePos(relPos, saveUndoRedo);
    ((VectorPath*)mParent)->updateDrawGradients();
}

void GradientPoint::moveByRel(const QPointF &relTranslatione) {
    MovablePoint::moveByRel(relTranslatione);
    ((VectorPath*)mParent)->updateDrawGradients();
}

void GradientPoint::setColor(QColor fillColor) {
    mFillColor = fillColor;
    
}

void GradientPoint::drawSk(SkCanvas *canvas,
                           const SkScalar &invScale) {
    if(mHidden) {
        return;
    }

    SkPoint absPos = QPointFToSkPoint(getAbsolutePos());
    canvas->save();

    SkScalar scaledRadius = mRadius*invScale;

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SkColorSetARGBInline(mFillColor.alpha(),
                                        mFillColor.red(),
                                        mFillColor.green(),
                                        mFillColor.blue()));

    paint.setStyle(SkPaint::kFill_Style);
    canvas->drawCircle(absPos,
                       scaledRadius, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(invScale);
    canvas->drawCircle(absPos,
                       scaledRadius, paint);

    if(prp_isKeyOnCurrentFrame()) {
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawCircle(absPos,
                           scaledRadius*0.5, paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorBLACK);
        canvas->drawCircle(absPos,
                           scaledRadius*0.5, paint);
    }
    canvas->restore();
}
