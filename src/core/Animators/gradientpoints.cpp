#include "gradientpoints.h"
#include "MovablePoints/gradientpoint.h"
#include "skia/skqtconversions.h"
#include "Boxes/pathbox.h"

GradientPoints::GradientPoints(PathBox * const parent) :
    StaticComplexAnimator("gradient points"), mParent_k(parent) {

    setPointsHandler(enve::make_shared<PointsHandler>());

    mStartAnimator = enve::make_shared<QPointFAnimator>("point1");
    ca_addChild(mStartAnimator);
    mStartPoint = enve::make_shared<GradientPoint>(mStartAnimator.get(), mParent_k);
    mPointsHandler->appendPt(mStartPoint);

    mEndAnimator = enve::make_shared<QPointFAnimator>("point2");
    ca_addChild(mEndAnimator);

    mEndPoint = enve::make_shared<GradientPoint>(mEndAnimator.get(), mParent_k);
    mPointsHandler->appendPt(mEndPoint);

    mEnabled = false;
}

void GradientPoints::enable() {
    mEnabled = true;
}

void GradientPoints::setPositions(const QPointF &startPos,
                                  const QPointF &endPos) {
    mStartPoint->setRelativePos(startPos);
    mEndPoint->setRelativePos(endPos);
}

void GradientPoints::disable() {
    mEnabled = false;
}

void GradientPoints::drawCanvasControls(SkCanvas * const canvas,
                                        const CanvasMode mode,
                                        const float invScale) {
    if(mode != CanvasMode::pointTransform) return;
    if(mEnabled) {
        const SkPoint startPos = toSkPoint(mStartPoint->getAbsolutePos());
        const SkPoint endPos = toSkPoint(mEndPoint->getAbsolutePos());
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        paint.setStrokeWidth(1.5f*invScale);
        paint.setStyle(SkPaint::kStroke_Style);

        canvas->drawLine(startPos, endPos, paint);
        paint.setColor(SK_ColorWHITE);
        paint.setStrokeWidth(0.75f*invScale);
        canvas->drawLine(startPos, endPos, paint);
        Property::drawCanvasControls(canvas, mode, invScale);
    }
}

QPointF GradientPoints::getStartPointAtRelFrame(const int relFrame) {
    return mStartAnimator->getEffectiveValue(relFrame);
}

QPointF GradientPoints::getEndPointAtRelFrame(const int relFrame) {
    return mEndAnimator->getEffectiveValue(relFrame);
}

QPointF GradientPoints::getStartPointAtRelFrameF(const qreal relFrame) {
    return mStartAnimator->getEffectiveValue(relFrame);
}

QPointF GradientPoints::getEndPointAtRelFrameF(const qreal relFrame) {
    return mEndAnimator->getEffectiveValue(relFrame);
}

void GradientPoints::setColors(const QColor& startColor,
                               const QColor& endColor) {
    mStartPoint->setColor(startColor);
    mEndPoint->setColor(endColor);
}
