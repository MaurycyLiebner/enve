// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef AUTOTILEDSURFACE_H
#define AUTOTILEDSURFACE_H

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
    AutoTiledSurface(const AutoTiledSurface& other);
    AutoTiledSurface(AutoTiledSurface&& other);

    AutoTiledSurface& operator=(const AutoTiledSurface& other);
    AutoTiledSurface& operator=(AutoTiledSurface&& other);

    ~AutoTiledSurface();

    void swap(AutoTiledSurface& other) {
        mAutoTilesData.swap(other.mAutoTilesData);
    }

    void setPixelClamp(const QRect& pixRect);
    void loadBitmap(const SkBitmap &src);

    MyPaintRectangle paintPressEvent(MyPaintBrush * const brush,
                                     const QPointF& pos,
                                     const double dTime,
                                     const double pressure,
                                     const double xtilt,
                                     const double ytilt) const {
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
                                    const double dTime,
                                    const double pressure,
                                    const double xtilt,
                                    const double ytilt) const {
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

    void tileToBitmap(const int tx, const int ty, SkBitmap& bitmap) {
        mAutoTilesData.tileToBitmap(tx, ty, bitmap);
    }

    SkBitmap tileToBitmap(const int tx, const int ty) {
        return mAutoTilesData.tileToBitmap(tx, ty);
    }

    SkBitmap toBitmap(const QMargins& margin = QMargins()) const {
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

    void write(eWriteStream& dst) const {
        mAutoTilesData.write(dst);
    }

    void read(eReadStream& src) {
        mAutoTilesData.read(src);
    }

    void clear() { mAutoTilesData.clear(); }
private:
    static void sFree(MyPaintSurface *surface);

    static void sRequestStart(MyPaintTiledSurface *tiled_surface,
                              MyPaintTileRequest *request);

    static void sRequestEnd(MyPaintTiledSurface *tiled_surface,
                            MyPaintTileRequest *request);

    void free();
    void startRequest(MyPaintTileRequest * const request);
    void endRequest(MyPaintTileRequest * const request);
protected:
    MyPaintTiledSurface fParent;
    MyPaintSurface* const fMyPaintSurface;
    AutoTilesData mAutoTilesData;
};

#endif // AUTOTILEDSURFACE_H
