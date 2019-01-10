#include "boxpathpoint.h"
#include "Boxes/boundingbox.h"
#include "pointhelpers.h"
#include "Animators/transformanimator.h"

BoxPathPoint::BoxPathPoint(QPointFAnimator *associatedAnimator,
                           BoxTransformAnimator *box) :
    AnimatedPoint(associatedAnimator, box, TYPE_PIVOT_POINT, 7.) {}

void BoxPathPoint::startTransform() {
    MovablePoint::startTransform();
    mSavedAbsPos = getAbsolutePos();
    BoxTransformAnimator *boxTrans =
            GetAsPtr(mParentTransform_cv, BoxTransformAnimator);
    boxTrans->startPivotTransform();
}

void BoxPathPoint::finishTransform() {
    MovablePoint::finishTransform();
    BoxTransformAnimator *boxTrans =
            GetAsPtr(mParentTransform_cv, BoxTransformAnimator);
    boxTrans->finishPivotTransform();
}

void BoxPathPoint::moveByAbs(const QPointF &absTranslatione) {
    QPointF absPos = mSavedAbsPos + absTranslatione;
    BoxTransformAnimator *boxTrans =
            GetAsPtr(mParentTransform_cv, BoxTransformAnimator);
    boxTrans->getParentBox()->setPivotAbsPos(absPos);
}

void BoxPathPoint::drawSk(SkCanvas *canvas,
                          const SkScalar &invScale) {
    if(isHidden()) {
        return;
    }
    SkPoint absPos = qPointToSk(getAbsolutePos());
    if(mSelected) {
        drawOnAbsPosSk(canvas, absPos, invScale,
                       255, 255, 0);
    } else {
        drawOnAbsPosSk(canvas, absPos, invScale,
                       255, 255, 125);
    }

    canvas->save();
    canvas->translate(absPos.x(), absPos.y());
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    SkScalar scaledHalfRadius = qrealToSkScalar(mRadius)*invScale*0.5f;
    canvas->drawLine(-scaledHalfRadius, 0.f, scaledHalfRadius, 0.f, paint);
    canvas->drawLine(0.f, -scaledHalfRadius, 0.f, scaledHalfRadius, paint);
    canvas->restore();
}
