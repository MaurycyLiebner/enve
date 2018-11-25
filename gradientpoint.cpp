#include "gradientpoint.h"
#include "Boxes/vectorpath.h"
#include "Animators/animatorupdater.h"
#include "pointhelpers.h"

GradientPoint::GradientPoint(QPointFAnimator* associatedAnimator,
                             PathBox* parent) :
    PointAnimatorMovablePoint(associatedAnimator,
                              parent->getTransformAnimator(),
                              TYPE_GRADIENT_POINT) {
    mAssociatedAnimator_k->prp_setUpdater(
                SPtrCreate(DisplayedFillStrokeSettingsUpdater)(parent));
}

void GradientPoint::setRelativePos(const QPointF &relPos) {
    PointAnimatorMovablePoint::setRelativePos(relPos);
    BoxTransformAnimator* boxParent =
            SPtrGetAs(mParentTransform_cv, BoxTransformAnimator);
    SPtrGetAs(boxParent->getParentBox(), VectorPath)->updateDrawGradients();
}

void GradientPoint::moveByRel(const QPointF &relTranslatione) {
    PointAnimatorMovablePoint::moveByRel(relTranslatione);
    BoxTransformAnimator* boxParent =
            SPtrGetAs(mParentTransform_cv, BoxTransformAnimator);
    SPtrGetAs(boxParent->getParentBox(), VectorPath)->updateDrawGradients();
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

    SkScalar scaledRadius = static_cast<SkScalar>(mRadius)*invScale;

    SkPaint paint;
    paint.setAntiAlias(true);
    SkColor paintColor = QColorToSkColor(mFillColor);
    paint.setColor(paintColor);

    paint.setStyle(SkPaint::kFill_Style);
    canvas->drawCircle(absPos,
                       scaledRadius, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(1.5f*invScale);
    canvas->drawCircle(absPos,
                       scaledRadius, paint);
    paint.setColor(SK_ColorWHITE);
    paint.setStrokeWidth(0.75f*invScale);
    canvas->drawCircle(absPos,
                       scaledRadius, paint);

    if(mAssociatedAnimator_k->prp_isKeyOnCurrentFrame()) {
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawCircle(absPos,
                           scaledRadius*0.5f, paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorBLACK);
        canvas->drawCircle(absPos,
                           scaledRadius*0.5f, paint);
    }
    canvas->restore();
}
