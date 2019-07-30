#include "linkcanvasrenderdata.h"
#include "skia/skiahelpers.h"
#include "skia/skqtconversions.h"

void LinkCanvasRenderData::process() {
    updateGlobalRect();
    if(fOpacity < 0.001) return;

    const auto info = SkiaHelpers::getPremulRGBAInfo(fGlobalRect.width(),
                                                     fGlobalRect.height());
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas rasterCanvas(bitmap);
    rasterCanvas.translate(toSkScalar(-fGlobalRect.left()),
                           toSkScalar(-fGlobalRect.top()));

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

    fRenderedImage = SkiaHelpers::transferDataToSkImage(bitmap);
}
