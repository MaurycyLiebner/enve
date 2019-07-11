#include "layerboxrenderdata.h"

ContainerBoxRenderData::ContainerBoxRenderData(BoundingBox * const parentBoxT) :
    BoundingBoxRenderData(parentBoxT) {
    mDelayDataSet = true;
}

void ContainerBoxRenderData::transformRenderCanvas(SkCanvas &canvas) const {
    canvas.translate(toSkScalar(-fDrawPos.x()),
                     toSkScalar(-fDrawPos.y()));
    canvas.concat(toSkMatrix(fResolutionScale));
}

void ContainerBoxRenderData::updateRelBoundingRect() {
    BoundingBoxRenderData::updateRelBoundingRect();
    for(const auto &child : fChildrenRenderData) {
        fOtherGlobalRects << child->fGlobalBoundingRect;
    }
}

void ContainerBoxRenderData::drawSk(SkCanvas * const canvas) {
    for(const auto &child : fChildrenRenderData) {
        canvas->save();
        child->drawRenderedImageForParent(canvas);
        canvas->restore();
    }
}
