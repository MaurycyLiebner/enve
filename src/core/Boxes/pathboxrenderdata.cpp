#include "pathboxrenderdata.h"

PathBoxRenderData::PathBoxRenderData(BoundingBox * const parentBox) :
    BoxRenderData(parentBox) {}

void PathBoxRenderData::updateRelBoundingRect() {
    SkPath totalPath;
    totalPath.addPath(fFillPath);
    totalPath.addPath(fOutlinePath);
    fRelBoundingRect = toQRectF(totalPath.computeTightBounds());
}

QPointF PathBoxRenderData::getCenterPosition() {
    return toQRectF(fEditPath.getBounds()).center();
}

void PathBoxRenderData::drawSk(SkCanvas * const canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);

    if(!fFillPath.isEmpty()) {
        fPaintSettings.applyPainterSettingsSk(&paint);
        canvas->drawPath(fFillPath, paint);
    }
    if(!fOutlinePath.isEmpty()) {
        paint.setShader(nullptr);
        fStrokeSettings.applyPainterSettingsSk(&paint);
        canvas->drawPath(fOutlinePath, paint);
    }
}
