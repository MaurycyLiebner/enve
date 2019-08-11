#include "boxpathpoint.h"
#include "pointhelpers.h"
#include "Animators/transformanimator.h"

BoxPathPoint::BoxPathPoint(QPointFAnimator * const associatedAnimator,
                           BoxTransformAnimator * const boxTrans) :
    AnimatedPoint(associatedAnimator, TYPE_PIVOT_POINT) {
    setRadius(7);
    setTransform(boxTrans);
    setSelectionEnabled(false);
}

void BoxPathPoint::setRelativePos(const QPointF &relPos) {
    const auto bTrans = static_cast<BoxTransformAnimator*>(getTransform());
    bTrans->setPivotFixedTransform(relPos);
}

void BoxPathPoint::startTransform() {
    MovablePoint::startTransform();
    const auto bTrans = static_cast<BoxTransformAnimator*>(getTransform());
    bTrans->startPivotTransform();
}

void BoxPathPoint::finishTransform() {
    const auto bTrans = static_cast<BoxTransformAnimator*>(getTransform());
    bTrans->finishPivotTransform();
}

void BoxPathPoint::drawSk(SkCanvas * const canvas, const CanvasMode mode,
                          const float invScale, const bool keyOnCurrent,
                          const bool ctrlPressed) {
    Q_UNUSED(mode);
    Q_UNUSED(keyOnCurrent);
    Q_UNUSED(ctrlPressed);
    const SkPoint absPos = toSkPoint(getAbsolutePos());
    const SkColor fillCol = isSelected() ?
                SkColorSetRGB(255, 255, 0) :
                SkColorSetRGB(255, 255, 125);
    drawOnAbsPosSk(canvas, absPos, invScale, fillCol);

    canvas->save();
    canvas->translate(absPos.x(), absPos.y());
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    const float scaledHalfRadius = toSkScalar(getRadius()*0.5)*invScale;
    canvas->drawLine(-scaledHalfRadius, 0, scaledHalfRadius, 0, paint);
    canvas->drawLine(0, -scaledHalfRadius, 0, scaledHalfRadius, paint);
    canvas->restore();
}
