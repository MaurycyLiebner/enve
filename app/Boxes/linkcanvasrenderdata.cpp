#include "linkcanvasrenderdata.h"
#include "skia/skiahelpers.h"
#include "PixmapEffects/pixmapeffect.h"

void LinkCanvasRenderData::renderToImage() {
    if(fRenderedToImage) return;
    fRenderedToImage = true;
    if(fOpacity < 0.001) return;
    updateGlobalFromRelBoundingRect();

    const auto info = SkiaHelpers::getPremulBGRAInfo(
                qCeil(fGlobalBoundingRect.width()),
                qCeil(fGlobalBoundingRect.height()));
    fBitmapTMP.allocPixels(info);
    fBitmapTMP.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas rasterCanvas(fBitmapTMP);
    rasterCanvas.translate(toSkScalar(-fGlobalBoundingRect.left()),
                           toSkScalar(-fGlobalBoundingRect.top()));

    if(fClipToCanvas) {
        rasterCanvas.save();
        rasterCanvas.concat(toSkMatrix(fScaledTransform));
        SkPaint fillP;
        fillP.setAntiAlias(true);
        fillP.setColor(fBgColor);
        rasterCanvas.drawRect(toSkRect(fRelBoundingRect), fillP);
        rasterCanvas.restore();
    }

    drawSk(&rasterCanvas);
    if(fClipToCanvas) {
        rasterCanvas.save();
        rasterCanvas.concat(toSkMatrix(fScaledTransform));
        SkPaint paintT;
        paintT.setBlendMode(SkBlendMode::kDstIn);
        paintT.setColor(SK_ColorTRANSPARENT);
        paintT.setAntiAlias(true);
        SkPath path;
        path.addRect(toSkRect(fRelBoundingRect));
        path.toggleInverseFillType();
        rasterCanvas.drawPath(path, paintT);
        rasterCanvas.restore();
    }
    rasterCanvas.flush();

    if(!fRasterEffects.isEmpty()) {
        for(const auto& effect : fRasterEffects) {
            effect->applyEffectsSk(fBitmapTMP, fResolution);
        }
        clearPixmapEffects();
    }

    fBitmapTMP.setImmutable();
    fRenderedImage = SkImage::MakeFromBitmap(fBitmapTMP);
    fBitmapTMP.reset();
}
