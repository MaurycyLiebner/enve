#include "drawableautotiledsurface.h"

void drawableRequestStart(MyPaintTiledSurface *tiled_surface,
                          MyPaintTileRequest *request) {
    const auto self = reinterpret_cast<DrawableAutoTiledSurface*>(tiled_surface);
    self->stretchToTileImg(request->tx, request->ty);
    self->_startRequest(request);
}

DrawableAutoTiledSurface::DrawableAutoTiledSurface() {
    fParent.tile_request_start = drawableRequestStart;
}

void DrawableAutoTiledSurface::drawOnCanvas(SkCanvas * const canvas,
                                            const QRect &minSrc,
                                            const QPoint &dst,
                                            SkPaint * const paint) const {
    const auto tileRect = mAutoTilesData.pixRectToTileRect(minSrc);
    for(int tx = tileRect.left(); tx <= tileRect.right(); tx++) {
        const SkScalar drawX = dst.x() + tx*MYPAINT_TILE_SIZE;
        for(int ty = tileRect.top(); ty <= tileRect.bottom(); ty++) {
            const auto img = imageForTile(tx, ty);
            if(!img) continue;
            const SkScalar drawY = dst.y() + ty*MYPAINT_TILE_SIZE;
            canvas->drawImage(img, drawX, drawY, paint);
        }
    }
}
