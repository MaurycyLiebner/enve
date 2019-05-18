#include "layerboxrenderdata.h"

LayerBoxRenderData::LayerBoxRenderData(BoundingBox * const parentBoxT) :
    BoundingBoxRenderData(parentBoxT) {
    mDelayDataSet = true;
}

void LayerBoxRenderData::drawSk(SkCanvas * const canvas) {
    for(const auto &child : fChildrenRenderData) {
        canvas->save();
        child->drawRenderedImageForParent(canvas);
        canvas->restore();
    }
}

void LayerBoxRenderData::transformRenderCanvas(SkCanvas &canvas) const {
    canvas.translate(toSkScalar(-fGlobalBoundingRect.left()),
                     toSkScalar(-fGlobalBoundingRect.top()));
    canvas.concat(toSkMatrix(fResolutionScale));
}

void LayerBoxRenderData::updateRelBoundingRect() {
    SkPath boundingPaths;
    for(const auto &child : fChildrenRenderData) {
        if(child->fRelBoundingRect.isEmpty()) continue;
        SkPath childPath;
        childPath.addRect(toSkRect(child->fRelBoundingRect));
        childPath.transform(toSkMatrix(child->fRelTransform));
        boundingPaths.addPath(childPath);
        fOtherGlobalRects << child->fGlobalBoundingRect;
    }
    fRelBoundingRect = toQRectF(boundingPaths.computeTightBounds());
}
