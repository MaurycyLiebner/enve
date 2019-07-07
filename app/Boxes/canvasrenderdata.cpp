#include "canvasrenderdata.h"
#include "skia/skiahelpers.h"
#include "PixmapEffects/pixmapeffect.h"

CanvasRenderData::CanvasRenderData(BoundingBox * const parentBoxT) :
    ContainerBoxRenderData(parentBoxT) {}

void CanvasRenderData::updateGlobalFromRelBoundingRect() {
    fResolutionScale.reset();
    fResolutionScale.scale(fResolution, fResolution);
    fScaledTransform = fResolutionScale;
    fGlobalBoundingRect = fScaledTransform.mapRect(fRelBoundingRect);
    fixupGlobalBoundingRect();
}

void CanvasRenderData::updateRelBoundingRect() {
    fRelBoundingRect = QRectF(0, 0, fCanvasWidth, fCanvasHeight);
}
