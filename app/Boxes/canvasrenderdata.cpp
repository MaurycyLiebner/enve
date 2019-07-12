#include "canvasrenderdata.h"
#include "skia/skiahelpers.h"
#include "PixmapEffects/pixmapeffect.h"

CanvasRenderData::CanvasRenderData(BoundingBox * const parentBoxT) :
    ContainerBoxRenderData(parentBoxT) {}

void CanvasRenderData::updateGlobalRect() {
    fResolutionScale.reset();
    fResolutionScale.scale(fResolution, fResolution);
    fScaledTransform = fResolutionScale;
    const auto globalRectF = fScaledTransform.mapRect(fRelBoundingRect);
    setGlobalRect(globalRectF);
}

void CanvasRenderData::updateRelBoundingRect() {
    fRelBoundingRect = QRectF(0, 0, fCanvasWidth, fCanvasHeight);
}
