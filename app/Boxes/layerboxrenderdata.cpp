#include "layerboxrenderdata.h"

ContainerBoxRenderData::ContainerBoxRenderData(BoundingBox * const parentBoxT) :
    BoundingBoxRenderData(parentBoxT) {
    mDelayDataSet = true;
}

void ContainerBoxRenderData::transformRenderCanvas(SkCanvas &canvas) const {
    canvas.translate(toSkScalar(-fGlobalRect.x()),
                     toSkScalar(-fGlobalRect.y()));
    canvas.concat(toSkMatrix(fResolutionScale));
}

void ContainerBoxRenderData::updateRelBoundingRect() {
    BoundingBoxRenderData::updateRelBoundingRect();
    for(const auto &child : fChildrenRenderData) {
        fOtherGlobalRects << child->fGlobalRect;
    }
}

void ContainerBoxRenderData::drawSk(SkCanvas * const canvas) {
    for(const auto &child : fChildrenRenderData) {
        canvas->save();
        child->drawRenderedImageForParent(canvas);
        canvas->restore();
    }
}
