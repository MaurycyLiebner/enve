#include "boxesgrouprenderdata.h"

BoxesGroupRenderData::BoxesGroupRenderData(BoundingBox * const parentBoxT) :
    BoundingBoxRenderData(parentBoxT) {
    mDelayDataSet = true;
}

void BoxesGroupRenderData::drawSk(SkCanvas * const canvas) {
    for(const auto &child : fChildrenRenderData) {
        canvas->save();
        child->drawRenderedImageForParent(canvas);
        canvas->restore();
    }
}

void BoxesGroupRenderData::transformRenderCanvas(SkCanvas &canvas) const {
    canvas.concat(toSkMatrix(fResolutionScale));
}

void BoxesGroupRenderData::updateRelBoundingRect() {
    SkPath boundingPaths;
    for(const auto &child : fChildrenRenderData) {
        if(child->fRelBoundingRect.isEmpty()) continue;
        SkPath childPath;
        childPath.addRect(
                    toSkRect(child->fRelBoundingRect));
        childPath.transform(
                    toSkMatrix(child->fRelTransform));
        boundingPaths.addPath(childPath);
        fOtherGlobalRects << child->fGlobalBoundingRect;
    }
    fRelBoundingRect = toQRectF(boundingPaths.computeTightBounds());
}
