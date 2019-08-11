#include "gradientpoint.h"
#include "pointhelpers.h"
#include "Boxes/pathbox.h"
#include "PropertyUpdaters/displayedfillstrokesettingsupdater.h"
#include "Animators/transformanimator.h"

GradientPoint::GradientPoint(QPointFAnimator * const associatedAnimator,
                             PathBox* const parent) :
    AnimatedPoint(associatedAnimator, parent->getTransformAnimator(),
                  TYPE_GRADIENT_POINT) {}

void GradientPoint::setColor(const QColor &fillColor) {
    mFillColor = fillColor;
}

void GradientPoint::drawSk(SkCanvas * const canvas, const CanvasMode mode,
                           const float invScale, const bool keyOnCurrent,
                           const bool ctrlPressed) {
    Q_UNUSED(mode);
    Q_UNUSED(ctrlPressed);

    const SkPoint absPos = toSkPoint(getAbsolutePos());
    canvas->save();

    const float scaledRadius = toSkScalar(getRadius())*invScale;

    SkPaint paint;
    paint.setAntiAlias(true);
    const SkColor paintColor = toSkColor(mFillColor);
    paint.setColor(paintColor);

    paint.setStyle(SkPaint::kFill_Style);
    canvas->drawCircle(absPos, scaledRadius, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(1.5f*invScale);
    canvas->drawCircle(absPos, scaledRadius, paint);
    paint.setColor(SK_ColorWHITE);
    paint.setStrokeWidth(0.75f*invScale);
    canvas->drawCircle(absPos, scaledRadius, paint);

    if(keyOnCurrent) {
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawCircle(absPos, scaledRadius*0.5f, paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorBLACK);
        canvas->drawCircle(absPos, scaledRadius*0.5f, paint);
    }
    canvas->restore();
}
