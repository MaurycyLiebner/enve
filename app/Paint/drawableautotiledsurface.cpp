#include "drawableautotiledsurface.h"

DrawableAutoTiledSurface::DrawableAutoTiledSurface() {}

void DrawableAutoTiledSurface::drawOnCanvas(SkCanvas * const canvas,
                                            const QRect &minPixSrc,
                                            const QPoint &dst,
                                            SkPaint * const paint) const {
    const QRect maxRect = tileBoundingRect();
    const QRect tileSrc = pixRectToTileRect(minPixSrc);
    if(!tileSrc.intersects(maxRect)) return;
    const auto tileRect = tileSrc.intersected(maxRect);
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
