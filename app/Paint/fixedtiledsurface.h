#ifndef FIXEDTILEDSURFACE_H
#define FIXEDTILEDSURFACE_H

#include <mypaint-config.h>
#include <mypaint-glib-compat.h>
#include <mypaint-tiled-surface.h>
#include <mypaint-brush.h>
#include <QPointF>
#include "skia/skiaincludes.h"
#include "smartPointers/stdselfref.h"
#include "pointhelpers.h"
#include "pathoperations.h"
#include "brushstroke.h"

struct FixedTiledSurface {
    friend struct BrushStroke;
    friend struct BrushStrokeSet;
    ~FixedTiledSurface();
    void _free() {
        mypaint_tiled_surface_destroy(&fParent);

        if(fTileBuffer) free(fTileBuffer);
        if(fNullTile) free(fNullTile);
    }
    void _startRequest(MyPaintTileRequest * const request);
    void _endRequest(MyPaintTileRequest * const request);

    bool initialize(const int width, const int height);
    bool resize(const int width, const int height);


    void paintPressEvent(MyPaintBrush * const brush,
                         const QPointF& pos,
                         const double dTime,
                         const double pressure,
                         const double xtilt,
                         const double ytilt) {
        mypaint_brush_reset(brush);
        mypaint_brush_new_stroke(brush);

        mypaint_surface_begin_atomic(fMyPaintSurface);
        mypaint_brush_stroke_to(brush,
                                fMyPaintSurface,
                                static_cast<float>(pos.x()),
                                static_cast<float>(pos.y()),
                                static_cast<float>(pressure),
                                static_cast<float>(xtilt),
                                static_cast<float>(ytilt),
                                dTime);
        MyPaintRectangle roi;
        mypaint_surface_end_atomic(fMyPaintSurface, &roi);
        updateImage(QRect(roi.x, roi.y, roi.width, roi.height));
    }

    void paintMoveEvent(MyPaintBrush * const brush,
                        const QPointF &pos,
                        const double dTime,
                        const double pressure,
                        const double xtilt,
                        const double ytilt) {
        mypaint_surface_begin_atomic(fMyPaintSurface);
        mypaint_brush_stroke_to(brush,
                                fMyPaintSurface,
                                static_cast<float>(pos.x()),
                                static_cast<float>(pos.y()),
                                static_cast<float>(pressure),
                                static_cast<float>(xtilt),
                                static_cast<float>(ytilt),
                                dTime);
        MyPaintRectangle roi;
        mypaint_surface_end_atomic(fMyPaintSurface, &roi);
        updateImage(QRect(roi.x, roi.y, roi.width, roi.height));
    }

    void draw(SkCanvas * const canvas,
              const SkScalar left, const SkScalar top,
              const SkPaint * const paint = nullptr) {
        canvas->drawBitmap(fBitmap, left, top, paint);
    }

    void loadBitmap(const SkBitmap &bitmap);

    void execute(MyPaintBrush * const brush,
                 BrushStrokeSet& set) {
        QRect updateRect = set.execute(brush, fMyPaintSurface, 5);
        updateImage(updateRect);
    }
protected:
    MyPaintTiledSurface fParent;
    MyPaintSurface* fMyPaintSurface = nullptr;

    size_t fBufferSize; // Size (in bytes) of the whole surface
    size_t fTileSize; // Size (in bytes) of single tile
    uint16_t* fTileBuffer = nullptr; // Stores tiles in a linear chunk of memory (16bpc RGBA)
    uint16_t* fNullTile = nullptr; // Single tile that we hand out and ignore writes to
    int fTilesWidth; // width in tiles
    int fTilesHeight; // height in tiles
    int fWidth; // width in pixels
    int fHeight; // height in pixels
    int fRealWidth;
    int fRealHeight;
    SkBitmap fBitmap;

    void updateImage(const QRect& roi) {
        if(roi.isNull() || !roi.isValid()) return;
        if(roi.width() <= 0) return;
        if(roi.height() <= 0) return;
        if(roi.x() + roi.width() < 0) return;
        if(roi.y() + roi.height() < 0) return;
        if(roi.x() >= fRealWidth) return;
        if(roi.y() >= fRealHeight) return;
        const int tileSize = fParent.tile_size;
        const int tilesWidth = fTilesWidth;
        const int tilesHeight = fTilesHeight;

        int minTileX = std::max(0, roi.x()/tileSize);
        int minTileY = std::max(0, roi.y()/tileSize);
        int maxTileX = qCeil(static_cast<double>(roi.x() + roi.width())/tileSize);
        int maxTileY = qCeil(static_cast<double>(roi.y() + roi.height())/tileSize);
        maxTileX = std::min(tilesWidth, maxTileX);
        maxTileY = std::min(tilesHeight, maxTileY);

        for(int i = minTileX; i < maxTileX; i++) {
            for(int j = minTileY; j < maxTileY; j++) {
                updateImage(i, j);
            }
        }
    }

    void updateImage(const int tx, const int ty);
    void resetNullTile();
};

#endif // FIXEDTILEDSURFACE_H
