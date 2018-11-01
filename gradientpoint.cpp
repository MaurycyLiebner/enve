#include "gradientpoint.h"
#include "Boxes/vectorpath.h"
#include "Animators/animatorupdater.h"
#include "pointhelpers.h"

GradientPoint::GradientPoint(PathBox *parent) :
    PointAnimator(parent->getTransformAnimator(), TYPE_GRADIENT_POINT) {
    prp_setUpdater(new DisplayedFillStrokeSettingsUpdater(parent));
}

void GradientPoint::setRelativePos(const QPointF &relPos) {
    PointAnimator::setRelativePos(relPos);
    ((VectorPath*)(((BoxTransformAnimator*)mParent)->getParentBox()))->updateDrawGradients();
}

void GradientPoint::moveByRel(const QPointF &relTranslatione) {
    PointAnimator::moveByRel(relTranslatione);
    ((VectorPath*)(((BoxTransformAnimator*)mParent)->getParentBox()))->updateDrawGradients();
}

void GradientPoint::setColor(const QColor &fillColor) {
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
    paint.setColor(SkColorSetARGB(mFillColor.alpha(),
                                        mFillColor.red(),
                                        mFillColor.green(),
                                        mFillColor.blue()));

    paint.setStyle(SkPaint::kFill_Style);
    canvas->drawCircle(absPos,
                       scaledRadius, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(1.5*invScale);
    canvas->drawCircle(absPos,
                       scaledRadius, paint);
    paint.setColor(SK_ColorWHITE);
    paint.setStrokeWidth(0.75*invScale);
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
