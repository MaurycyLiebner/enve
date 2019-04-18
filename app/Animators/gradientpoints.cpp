#include "gradientpoints.h"
#include "MovablePoints/gradientpoint.h"
#include "skia/skqtconversions.h"
#include "Boxes/pathbox.h"

GradientPoints::GradientPoints(PathBox *parentT) :
    ComplexAnimator("gradient points"), mParent_k(parentT) {

    mStartAnimator = SPtrCreate(QPointFAnimator)("point1");
    ca_addChildAnimator(mStartAnimator);
    mStartPoint = SPtrCreate(GradientPoint)(mStartAnimator.get(), mParent_k);

    mEndAnimator = SPtrCreate(QPointFAnimator)("point2");
    ca_addChildAnimator(mEndAnimator);
    mEndPoint = SPtrCreate(GradientPoint)(mEndAnimator.get(), mParent_k);

    mEnabled = false;
}

void GradientPoints::enable() {
    if(mEnabled) {
        return;
    }
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

void GradientPoints::drawGradientPointsSk(SkCanvas *canvas,
                                          const CanvasMode& mode,
                                          const SkScalar &invScale,
                                          const bool& keyOnCurrent) {
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
        mStartPoint->drawSk(canvas, mode, invScale, keyOnCurrent);
        mEndPoint->drawSk(canvas, mode, invScale, keyOnCurrent);
    }
}

MovablePoint *GradientPoints::getPointAt(const QPointF &absPos,
                                         const qreal &canvasScaleInv) {
    if(mEnabled) {
        if(mStartPoint->isPointAtAbsPos(absPos, canvasScaleInv) ) {
            return mStartPoint.get();
        } else if(mEndPoint->isPointAtAbsPos(absPos, canvasScaleInv) ) {
            return mEndPoint.get();
        }
    }
    return nullptr;
}

QPointF GradientPoints::getStartPointAtRelFrame(const int &relFrame) {
    return mStartAnimator->getEffectiveValueAtRelFrame(relFrame);
}

QPointF GradientPoints::getEndPointAtRelFrame(const int &relFrame) {
    return mEndAnimator->getEffectiveValueAtRelFrame(relFrame);
}

QPointF GradientPoints::getStartPointAtRelFrameF(const qreal &relFrame) {
    return mStartAnimator->getEffectiveValueAtRelFrame(relFrame);
}

QPointF GradientPoints::getEndPointAtRelFrameF(const qreal &relFrame) {
    return mEndAnimator->getEffectiveValueAtRelFrame(relFrame);
}

void GradientPoints::setColors(const QColor& startColor,
                               const QColor& endColor) {
    mStartPoint->setColor(startColor);
    mEndPoint->setColor(endColor);
}
