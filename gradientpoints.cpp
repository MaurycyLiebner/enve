#include "gradientpoints.h"
#include "gradientpoint.h"
#include "skqtconversions.h"

GradientPoints::GradientPoints() : ComplexAnimator()
{
    prp_setName("gradient points");
}

GradientPoints::~GradientPoints() {
}

void GradientPoints::initialize(PathBox *parentT)
{
    parent = parentT;
    startPoint = new GradientPoint(parent);
    ca_addChildAnimator(startPoint );
    startPoint->prp_setName("point1");
    endPoint = new GradientPoint(parent);
    endPoint->prp_setName("point2");
    ca_addChildAnimator(endPoint);
    enabled = false;
}

void GradientPoints::enable() {
    if(enabled) {
        return;
    }
    enabled = true;
}

void GradientPoints::setPositions(const QPointF &startPos,
                                  const QPointF &endPos) {
    startPoint->setRelativePos(startPos);
    endPoint->setRelativePos(endPos);
}

void GradientPoints::disable()
{
    enabled = false;
}

void GradientPoints::drawGradientPointsSk(SkCanvas *canvas,
                                          const SkScalar &invScale) {
    if(enabled) {
        SkPoint startPos = QPointFToSkPoint(startPoint->getAbsolutePos());
        SkPoint endPos = QPointFToSkPoint(endPoint->getAbsolutePos());
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        paint.setStrokeWidth(1.5*invScale);
        paint.setStyle(SkPaint::kStroke_Style);

        canvas->drawLine(startPos, endPos, paint);
        paint.setColor(SK_ColorWHITE);
        paint.setStrokeWidth(0.75*invScale);
        canvas->drawLine(startPos, endPos, paint);
        startPoint->drawSk(canvas, invScale);
        endPoint->drawSk(canvas, invScale);
    }
}

MovablePoint *GradientPoints::qra_getPointAt(const QPointF &absPos,
                                             const qreal &canvasScaleInv) {
    if(enabled) {
        if(startPoint->isPointAtAbsPos(absPos, canvasScaleInv) ) {
            return startPoint;
        } else if (endPoint->isPointAtAbsPos(absPos, canvasScaleInv) ){
            return endPoint;
        }
    }
    return NULL;
}

QPointF GradientPoints::getStartPointAtRelFrame(const int &relFrame) {
    return startPoint->getCurrentEffectivePointValueAtRelFrame(relFrame);
}

QPointF GradientPoints::getEndPointAtRelFrame(const int &relFrame) {
    return endPoint->getCurrentEffectivePointValueAtRelFrame(relFrame);
}

void GradientPoints::setColors(QColor startColor, QColor endColor) {
    startPoint->setColor(startColor);
    endPoint->setColor(endColor);
}
