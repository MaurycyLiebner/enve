#include "layerboxrenderdata.h"
#include "skia/skqtconversions.h"

ContainerBoxRenderData::ContainerBoxRenderData(BoundingBox * const parentBoxT) :
    BoxRenderData(parentBoxT) {
    mDelayDataSet = true;
}

void ContainerBoxRenderData::transformRenderCanvas(SkCanvas &canvas) const {
    canvas.translate(toSkScalar(-fGlobalRect.x()),
                     toSkScalar(-fGlobalRect.y()));
}

void ContainerBoxRenderData::updateRelBoundingRect() {
    SkPath boundingPaths;
    for(const auto &child : fChildrenRenderData) {
        SkPath childPath;
        childPath.addRect(toSkRect(child->fRelBoundingRect));
        childPath.transform(toSkMatrix(child->fRelTransform));
        boundingPaths.addPath(childPath);

        fOtherGlobalRects << child->fGlobalRect;
    }
    fRelBoundingRect = toQRectF(boundingPaths.computeTightBounds());
}

void ContainerBoxRenderData::drawSk(SkCanvas * const canvas) {
    for(const auto &child : fChildrenRenderData) {
        canvas->save();
        child->drawRenderedImageForParent(canvas);
        canvas->restore();
    }
}
