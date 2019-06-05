#include "pathpivot.h"
#include "canvas.h"
#include "pointhelpers.h"
#include "Animators/transformanimator.h"

PathPivot::PathPivot(const Canvas * const parent) :
    NonAnimatedMovablePoint(parent->getTransformAnimator(),
                            TYPE_PIVOT_POINT),
    mCanvas(parent) {
    setRadius(7);
}

void PathPivot::drawSk(SkCanvas * const canvas,
                       const CanvasMode &mode,
                       const SkScalar invScale,
                       const bool keyOnCurrent) {
    Q_UNUSED(keyOnCurrent);
    const SkPoint absPos = toSkPoint(getAbsolutePos());
    if(isVisible(mode)) {
        const SkColor fillCol = isSelected() ?
                    SkColorSetRGB(0, 255, 0) :
                    SkColorSetRGB(125, 255, 125);
        const SkPoint absPos = toSkPoint(getAbsolutePos());
        drawOnAbsPosSk(canvas, absPos, invScale, fillCol);
    }

    canvas->save();
    canvas->translate(absPos.x(), absPos.y());
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    const SkScalar scaledHalfRadius = toSkScalar(getRadius()*0.5)*invScale;
    canvas->drawLine(-scaledHalfRadius, 0, scaledHalfRadius, 0, paint);
    canvas->drawLine(0, -scaledHalfRadius, 0, scaledHalfRadius, paint);
    canvas->restore();
}

bool PathPivot::isVisible(const CanvasMode &mode) const {
    if(mCanvas->getPivotLocal()) return false;
    if(mode == MOVE_POINT) return !mCanvas->isPointSelectionEmpty();
    else if(mode == MOVE_BOX) return !mCanvas->isBoxSelectionEmpty();
    return false;
}
