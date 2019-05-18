#include "boxesgrouprenderdata.h"

BoxesGroupRenderData::BoxesGroupRenderData(BoundingBox * const parentBoxT) :
    BoundingBoxRenderData(parentBoxT) {
    mDelayDataSet = true;
}

void BoxesGroupRenderData::drawRenderedImageForParent(SkCanvas * const canvas) {
    if(fPaintOnImage) {
        return BoundingBoxRenderData::drawRenderedImageForParent(canvas);
    }
    if(fOpacity < 0.001) return;
//    const SkScalar invScale = toSkScalar(1/fResolution);
//    canvas->scale(invScale, invScale);
//    canvas->concat(toSkMatrix(fRenderTransform));
    SkPaint paint;
    paint.setAlpha(static_cast<U8CPU>(qRound(fOpacity*2.55)));
    paint.setBlendMode(fBlendMode);
    canvas->saveLayer(nullptr, &paint);
    for(const auto& child : fChildrenRenderData) {
        child->drawRenderedImageForParent(canvas);
    }
    canvas->restore();
}

void BoxesGroupRenderData::processTask() {
    if(fPaintOnImage) {
        return BoundingBoxRenderData::processTask();
    }
    if(fRenderedToImage) return;
    fRenderedToImage = true;
    if(fOpacity < 0.001) return;
    updateGlobalFromRelBoundingRect();
}

void BoxesGroupRenderData::copyFrom(BoundingBoxRenderData *src) {
    return BoundingBoxRenderData::copyFrom(src);
    const auto groupData = GetAsPtr(src, BoxesGroupRenderData);
    fChildrenRenderData = groupData->fChildrenRenderData;
    fPaintOnImage = groupData->fPaintOnImage;
}

void BoxesGroupRenderData::drawSk(SkCanvas * const canvas) {
    for(const auto &child : fChildrenRenderData) {
        canvas->save();
        child->drawRenderedImageForParent(canvas);
        canvas->restore();
    }
}

void BoxesGroupRenderData::transformRenderCanvas(SkCanvas &canvas) const {
    canvas.translate(toSkScalar(-fGlobalBoundingRect.left()),
                     toSkScalar(-fGlobalBoundingRect.top()));
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
