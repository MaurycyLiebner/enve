#include "drawableautotiledsurface.h"

DrawableAutoTiledSurface::DrawableAutoTiledSurface() :
    HDDCachable(false) {}

void DrawableAutoTiledSurface::drawOnCanvas(SkCanvas * const canvas,
                                            const QPoint &dst,
                                            const QRect * const minPixSrc,
                                            SkPaint * const paint) const {
    const QRect maxRect = tileBoundingRect();
    QRect tileRect;
    if(minPixSrc) {
        const QRect tileSrc = pixRectToTileRect(*minPixSrc);
        if(!tileSrc.intersects(maxRect)) return;
        tileRect = tileSrc.intersected(maxRect);
    } else tileRect = maxRect;
    for(int tx = tileRect.left(); tx <= tileRect.right(); tx++) {
        const SkScalar drawX = dst.x() + tx*TILE_SIZE;
        for(int ty = tileRect.top(); ty <= tileRect.bottom(); ty++) {
            const auto img = imageForTile(tx, ty);
            if(!img) continue;
            const SkScalar drawY = dst.y() + ty*TILE_SIZE;
            canvas->drawImage(img, drawX, drawY, paint);
        }
    }
}
