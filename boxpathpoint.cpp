#include "boxpathpoint.h"
#include "Boxes/boundingbox.h"
#include "pointhelpers.h"

BoxPathPoint::BoxPathPoint(BoxTransformAnimator *box) :
    PointAnimator(box, TYPE_PIVOT_POINT, 7.) {
}

void BoxPathPoint::startTransform() {
    MovablePoint::startTransform();
    mSavedAbsPos = getAbsolutePos();
    ((BoxTransformAnimator*)mParent)->startPivotTransform();
}

void BoxPathPoint::finishTransform() {
    MovablePoint::finishTransform();
    ((BoxTransformAnimator*)mParent)->finishPivotTransform();
}

void BoxPathPoint::moveByAbs(const QPointF &absTranslatione) {
    QPointF absPos = mSavedAbsPos + absTranslatione;
    BoxTransformAnimator *boxTrans = (BoxTransformAnimator*)mParent;
    boxTrans->getParentBox()->setPivotAbsPos(absPos, false, false);
}

void BoxPathPoint::drawSk(SkCanvas *canvas,
                          const SkScalar &invScale) {
    if(isHidden()) {
        return;
    }
    SkPoint absPos = QPointFToSkPoint(getAbsolutePos());
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
    SkScalar scaledHalfRadius = mRadius*invScale*0.5;
    canvas->drawLine(-scaledHalfRadius, 0., scaledHalfRadius, 0., paint);
    canvas->drawLine(0, -scaledHalfRadius, 0., scaledHalfRadius, paint);
    canvas->restore();
}
