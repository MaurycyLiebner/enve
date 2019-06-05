#ifndef FIXEDTILEDSURFACE_H
#define FIXEDTILEDSURFACE_H

#include <mypaint-config.h>
#include <mypaint-glib-compat.h>
#include <mypaint-tiled-surface.h>
#include <mypaint-brush.h>
#include <QPointF>
#include "smartPointers/stdselfref.h"
#include "pointhelpers.h"
#include "pathoperations.h"
#include "autotilesdata.h"
#include "brushstroke.h"

struct AutoTiledSurface {
    friend struct BrushStroke;
    friend struct BrushStrokeSet;

    AutoTiledSurface();
    AutoTiledSurface(const AutoTiledSurface& other) = delete;
    AutoTiledSurface& operator=(const AutoTiledSurface& other) = delete;

    ~AutoTiledSurface();


    void deepCopy(const AutoTiledSurface& other) {
        mAutoTilesData.deepCopy(other.mAutoTilesData);
    }

    void swap(AutoTiledSurface& other) {
        mAutoTilesData.swap(other.mAutoTilesData);
    }

    void loadBitmap(const SkBitmap &src);

    void _free() {
        mypaint_tiled_surface_destroy(&fParent);
    }
    void _startRequest(MyPaintTileRequest * const request);
    void _endRequest(MyPaintTileRequest * const request);

    MyPaintRectangle paintPressEvent(MyPaintBrush * const brush,
                                     const QPointF& pos,
                                     const double& dTime,
                                     const double& pressure,
                                     const double& xtilt,
                                     const double& ytilt) const {
        mypaint_brush_reset(brush);
        mypaint_brush_new_stroke(brush);

        mypaint_surface_begin_atomic(fMyPaintSurface);
        mypaint_brush_stroke_to(brush, fMyPaintSurface,
                                pos.x(), pos.y(), pressure,
                                xtilt, ytilt, dTime);
        MyPaintRectangle roi;
        mypaint_surface_end_atomic(fMyPaintSurface, &roi);
        return roi;
    }

    MyPaintRectangle paintMoveEvent(MyPaintBrush * const brush,
                                    const QPointF &pos,
                                    const double& dTime,
                                    const double& pressure,
                                    const double& xtilt,
                                    const double& ytilt) const {
        mypaint_surface_begin_atomic(fMyPaintSurface);
        mypaint_brush_stroke_to(brush, fMyPaintSurface,
                                pos.x(), pos.y(), pressure,
                                xtilt, ytilt, dTime);
        MyPaintRectangle roi;
        mypaint_surface_end_atomic(fMyPaintSurface, &roi);
        return roi;
    }

    void execute(MyPaintBrush * const brush, BrushStrokeSet& set) {
        set.execute(brush, fMyPaintSurface, 5);
    }

    SkBitmap tileToBitmap(const int& tx, const int& ty) {
        return mAutoTilesData.tileToBitmap(tx, ty);
    }
    SkBitmap toBitmap(const int& margin = 0) const {
        return mAutoTilesData.toBitmap(margin);
    }

    QPoint zeroTilePos() const {
        return mAutoTilesData.zeroTilePos();
    }

    QRect pixelBoundingRect() const {
        return mAutoTilesData.pixelBoundingRect();
    }

    QRect tileBoundingRect() const {
        return mAutoTilesData.tileBoundingRect();
    }

    bool isEmpty() const { return mAutoTilesData.isEmpty(); }

    void write(QIODevice *target) {
        mAutoTilesData.write(target);
    }

    void read(QIODevice *target) {
        mAutoTilesData.read(target);
    }
protected:
    MyPaintTiledSurface fParent;
    MyPaintSurface* fMyPaintSurface = nullptr;

    AutoTilesData mAutoTilesData;
};

#endif // FIXEDTILEDSURFACE_H
