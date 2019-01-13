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

typedef qCubicSegment2D BrushStrokePath;
typedef qCubicSegment1D BrushPressureCurve;
typedef qCubicSegment1D BrushXTiltCurve;
typedef qCubicSegment1D BrushYTiltCurve;
typedef qCubicSegment1D BrushTimeCurve;

G_BEGIN_DECLS

/**
 * FixedTiledSurface:
 *
 * Simple #MyPaintTiledSurface subclass that implements a fixed sized #MyPaintSurface.
 * Only intended for testing and trivial use-cases, and to serve as an example of
 * how to implement a tiled surface subclass.
 */
struct FixedTiledSurface {
    friend struct BrushStroke;
    friend struct BrushStrokeSet;
    void _free() {
        mypaint_tiled_surface_destroy(&fParent);

        free(fTileBuffer);
        free(fNullTile);

        free(this);
    }
    void _startRequest(MyPaintTileRequest * const request);
    void _endRequest(MyPaintTileRequest * const request);

    bool initialize(const int& width, const int& height);
    bool resize(const int &width, const int &height);


    void paintPressEvent(MyPaintBrush * const brush,
                         const QPointF& pos,
                         const double& dTime,
                         const double& pressure,
                         const double& xtilt,
                         const double& ytilt) {
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
                                dTime,
                                static_cast<float>(1.),
                                static_cast<float>(0.));
        MyPaintRectangle roi;
        mypaint_surface_end_atomic(fMyPaintSurface, &roi);
        updateImage(roi);
    }

    void paintMoveEvent(MyPaintBrush * const brush,
                        const QPointF &pos,
                        const double& dTime,
                        const double& pressure,
                        const double& xtilt,
                        const double& ytilt) {
        mypaint_surface_begin_atomic(fMyPaintSurface);
        mypaint_brush_stroke_to(brush,
                                fMyPaintSurface,
                                static_cast<float>(pos.x()),
                                static_cast<float>(pos.y()),
                                static_cast<float>(pressure),
                                static_cast<float>(xtilt),
                                static_cast<float>(ytilt),
                                dTime,
                                static_cast<float>(1.),
                                static_cast<float>(0.));
        MyPaintRectangle roi;
        mypaint_surface_end_atomic(fMyPaintSurface, &roi);
        updateImage(roi);
    }

    void draw(SkCanvas * const canvas,
              const SkScalar& left, const SkScalar& top,
              const SkPaint * const paint = nullptr) {
        canvas->drawBitmap(fBitmap, left, top, paint);
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

    void updateImage(const MyPaintRectangle& roi) {
        if(roi.width <= 0) return;
        if(roi.height <= 0) return;
        if(roi.x + roi.width < 0) return;
        if(roi.y + roi.height < 0) return;
        if(roi.x >= fRealWidth) return;
        if(roi.y >= fRealHeight) return;
        const int tileSize = fParent.tile_size;
        const int tilesWidth = fTilesWidth;
        const int tilesHeight = fTilesHeight;

        int minTileX = std::max(0, roi.x/tileSize);
        int minTileY = std::max(0, roi.y/tileSize);
        int maxTileX = qCeil(static_cast<double>(roi.x + roi.width)/tileSize);
        int maxTileY = qCeil(static_cast<double>(roi.y + roi.height)/tileSize);
        maxTileX = std::min(tilesWidth, maxTileX);
        maxTileY = std::min(tilesHeight, maxTileY);

        for(int i = minTileX; i < maxTileX; i++) {
            for(int j = minTileY; j < maxTileY; j++) {
                updateImage(i, j);
            }
        }
    }

    void updateImage(const int& tx, const int& ty);
    void resetNullTile();
};
#define DefaultMoveStrokePressure(press) \
    BrushPressureCurve{press, press, press, press}
#define DefaultPressStrokePressure(iniPress, press, hardness) \
    BrushPressureCurve{iniPress, iniPress*(1 - hardness) + press*hardness, press, press}
#define DefaultTiltCurve \
    qCubicSegment1D{0, 0, 0, 0}
#define DefaultTimeCurve \
    qCubicSegment1D{0.1, 0.1, 0.1, 0.1}
struct BrushStroke {
    friend struct BrushStrokeSet;
    BrushStrokePath fStrokePath;
    BrushPressureCurve fPressure;
    BrushXTiltCurve fXTilt;
    BrushYTiltCurve fYTilt;
    BrushTimeCurve fTimeCurve;
private:
    void execute(MyPaintBrush * const brush,
                 FixedTiledSurface * const surface,
                 const bool& press,
                 MyPaintRectangle * const roiTotal,
                 const double& dLen) const {
        MyPaintRectangle roiSum;
        if(press) {
            executePress(brush, surface, &roiSum);
        }
        const double totalLength = gCubicLength(fStrokePath);
        double lastT = 0;
        for(double len = dLen; len < totalLength; len += dLen) {
            double t = gCubicTimeAtLength(fStrokePath, len);
            MyPaintRectangle roi;
            executeMove(brush, surface, t, lastT, &roi);
            mypaint_rectangle_expand_to_include_point(
                        &roiSum, roi.x, roi.y);
            mypaint_rectangle_expand_to_include_point(
                        &roiSum, roi.x + roi.width,
                        roi.y + roi.height);
            lastT = t;
        }
        mypaint_rectangle_expand_to_include_point(
                    roiTotal, roiSum.x, roiSum.y);
        mypaint_rectangle_expand_to_include_point(
                    roiTotal, roiSum.x + roiSum.width,
                    roiSum.y + roiSum.height);
    }

    void executeMove(MyPaintBrush * const brush,
                     FixedTiledSurface * const surface,
                     const double& t, const double& lastT,
                     MyPaintRectangle * const roi) const {
        QPointF pos = gCubicValueAtT(fStrokePath, t);
        qreal pressure = gCubicValueAtT(fPressure, t);
        qreal xTilt = gCubicValueAtT(fXTilt, t);
        qreal yTilt = gCubicValueAtT(fYTilt, t);
        qreal time = gCubicValueAtT(fTimeCurve, t);

        mypaint_surface_begin_atomic(surface->fMyPaintSurface);
        mypaint_brush_stroke_to(brush,
                                surface->fMyPaintSurface,
                                static_cast<float>(pos.x()),
                                static_cast<float>(pos.y()),
                                static_cast<float>(pressure),
                                static_cast<float>(xTilt),
                                static_cast<float>(yTilt),
                                time*(t - lastT),
                                static_cast<float>(1.),
                                static_cast<float>(0.));
        mypaint_surface_end_atomic(surface->fMyPaintSurface, roi);
    }

    void executePress(MyPaintBrush * const brush,
                      FixedTiledSurface * const surface,
                      MyPaintRectangle * const roi) const {
        QPointF pos = gCubicValueAtT(fStrokePath, 0);
        qreal pressure = gCubicValueAtT(fPressure, 0);
        qreal xTilt = gCubicValueAtT(fXTilt, 0);
        qreal yTilt = gCubicValueAtT(fYTilt, 0);
        //qreal time = gCubicValueAtT(fTimeCurve, t);

        mypaint_brush_reset(brush);
        mypaint_brush_new_stroke(brush);

        mypaint_surface_begin_atomic(surface->fMyPaintSurface);
        mypaint_brush_stroke_to(brush,
                                surface->fMyPaintSurface,
                                static_cast<float>(pos.x()),
                                static_cast<float>(pos.y()),
                                static_cast<float>(pressure),
                                static_cast<float>(xTilt),
                                static_cast<float>(yTilt),
                                1,
                                static_cast<float>(1.),
                                static_cast<float>(0.));
        mypaint_surface_end_atomic(surface->fMyPaintSurface, roi);
    }
};

struct BrushStrokeSet {
    static BrushStrokeSet fromSkPath(const SkPath& path) {
        BrushStrokeSet set;
        auto segs = gPathToQCubicSegs2D(path);
        bool first = true;
        foreach(const auto& seg, segs) {
            if(first) {
                first = false;
                set.fStrokes << BrushStroke{seg,
                                 DefaultPressStrokePressure(0, 0.8, 0.5),
                                 DefaultTiltCurve,
                                 DefaultTiltCurve,
                                 DefaultTimeCurve};
                continue;
            }
            set.fStrokes << BrushStroke{seg,
                             DefaultMoveStrokePressure(0.8),
                             DefaultTiltCurve,
                             DefaultTiltCurve,
                             DefaultTimeCurve};
        }
        return set;
    }

    void execute(MyPaintBrush * const brush,
                 FixedTiledSurface * const surface,
                 const double& dLen) const {
        if(fStrokes.isEmpty()) return;
        MyPaintRectangle roiSum;
        fStrokes.first().execute(brush, surface, true, &roiSum, dLen);
        foreach(const auto& stroke, fStrokes) {
            stroke.execute(brush, surface, false, &roiSum, dLen);
        }
        surface->updateImage(roiSum);
    }
    QList<BrushStroke> fStrokes;
};

G_END_DECLS

#endif // FIXEDTILEDSURFACE_H
