#include "canvasrenderdata.h"
#include "skia/skiahelpers.h"
#include "PixmapEffects/pixmapeffect.h"

CanvasRenderData::CanvasRenderData(BoundingBox * const parentBoxT) :
    BoxesGroupRenderData(parentBoxT) {}

void CanvasRenderData::renderToImage() {
    if(fRenderedToImage) return;
    fRenderedToImage = true;

    const auto info = SkiaHelpers::getPremulBGRAInfo(
                qCeil(fCanvasWidth), qCeil(fCanvasHeight));
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(fBgColor);
    SkCanvas rasterCanvas(bitmap);
    //rasterCanvas->clear(bgColor);

    drawSk(&rasterCanvas);
    rasterCanvas.flush();

    if(!fRasterEffects.isEmpty()) {
        for(const auto& effect : fRasterEffects) {
            effect->applyEffectsSk(bitmap, fResolution);
        }
        clearPixmapEffects();
    }

    bitmap.setImmutable();
    fRenderedImage = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
}

void CanvasRenderData::drawSk(SkCanvas * const canvas) {
    canvas->save();

    canvas->scale(toSkScalar(fResolution), toSkScalar(fResolution));
    for(const auto &renderData : fChildrenRenderData) {
        canvas->save();
        renderData->drawRenderedImageForParent(canvas);
        canvas->restore();
    }

    canvas->restore();
}

void CanvasRenderData::updateRelBoundingRect() {
    fRelBoundingRect = QRectF(0, 0, fCanvasWidth, fCanvasHeight);
}
