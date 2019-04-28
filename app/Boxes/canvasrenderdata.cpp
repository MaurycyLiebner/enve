#include "canvasrenderdata.h"
#include "skia/skiahelpers.h"
#include "PixmapEffects/pixmapeffect.h"

CanvasRenderData::CanvasRenderData(BoundingBox * const parentBoxT) :
    BoxesGroupRenderData(parentBoxT) {}

void CanvasRenderData::processTask() {
    if(fRenderedToImage) return;
    fRenderedToImage = true;

    const auto info = SkiaHelpers::getPremulBGRAInfo(
                qCeil(fCanvasWidth), qCeil(fCanvasHeight));
    fBitmapTMP.allocPixels(info);
    fBitmapTMP.eraseColor(fBgColor);
    SkCanvas rasterCanvas(fBitmapTMP);
    //rasterCanvas->clear(bgColor);

    drawSk(&rasterCanvas);
    rasterCanvas.flush();

    uint8_t* src = static_cast<uint8_t*>(fBitmapTMP.getPixels());
    const int iMax = fBitmapTMP.width()*fBitmapTMP.height();
    for(int i = 0; i < iMax; i++) {
        const uint8_t newAlpha = 255 - *src;
        *(src++) = 0;
        *(src++) = 0;
        *(src++) = 0;
        *(src++) = newAlpha;
    }

    if(!fRasterEffects.isEmpty()) {
        for(const auto& effect : fRasterEffects) {
            effect->applyEffectsSk(fBitmapTMP, fResolution);
        }
        clearPixmapEffects();
    }

    rasterCanvas.drawColor(SK_ColorWHITE, SkBlendMode::kDstOver);

    fRenderedImage = SkiaHelpers::transferDataToSkImage(fBitmapTMP);
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
