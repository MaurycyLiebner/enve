#include "layerboxrenderdata.h"

GroupBoxRenderData::GroupBoxRenderData(BoundingBox * const parentBoxT) :
    BoundingBoxRenderData(parentBoxT) {
    mDelayDataSet = true;
}

void GroupBoxRenderData::transformRenderCanvas(SkCanvas &canvas) const {
    canvas.translate(toSkScalar(-fGlobalBoundingRect.left()),
                     toSkScalar(-fGlobalBoundingRect.top()));
    canvas.concat(toSkMatrix(fResolutionScale));
}

void GroupBoxRenderData::updateRelBoundingRect() {
    BoundingBoxRenderData::updateRelBoundingRect();
    for(const auto &child : fChildrenRenderData) {
        fOtherGlobalRects << child->fGlobalBoundingRect;
    }
}

LayerBoxRenderData::LayerBoxRenderData(BoundingBox * const parentBoxT) :
    GroupBoxRenderData(parentBoxT) {}

void LayerBoxRenderData::drawSk(SkCanvas * const canvas) {
    for(const auto &child : fChildrenRenderData) {
        canvas->save();
        child->drawRenderedImageForParent(canvas);
        canvas->restore();
    }
}
