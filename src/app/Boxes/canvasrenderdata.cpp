#include "canvasrenderdata.h"
#include "skia/skiahelpers.h"

CanvasRenderData::CanvasRenderData(BoundingBox * const parentBoxT) :
    ContainerBoxRenderData(parentBoxT) {}

void CanvasRenderData::updateGlobalRect() {
    fResolutionScale.reset();
    fResolutionScale.scale(fResolution, fResolution);
    fScaledTransform = fResolutionScale;
    const auto globalRectF = fScaledTransform.mapRect(fRelBoundingRect);
    const QPoint pos(qFloor(globalRectF.left()),
                     qFloor(globalRectF.top()));
    const QSize size(qCeil(globalRectF.width()),
                     qCeil(globalRectF.height()));
    fGlobalRect = QRect(pos, size);
    //setBaseGlobalRect(globalRectF);
}

void CanvasRenderData::updateRelBoundingRect() {
    fRelBoundingRect = QRectF(0, 0, fCanvasWidth, fCanvasHeight);
}
