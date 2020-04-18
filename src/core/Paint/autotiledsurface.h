// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#include <QPointF>

#include "smartPointers/stdselfref.h"
#include "libmypaintincludes.h"
#include "pointhelpers.h"
#include "pathoperations.h"
#include "autotilesdata.h"
#include "brushstrokeset.h"

class CORE_EXPORT AutoTiledSurfaceBase {
    friend struct BrushStroke;
    friend struct BrushStrokeSet;
public:
    using Request = MyPaintTileRequestStartFunction;
    using TileCreator = std::function<stdsptr<Tile>(const size_t&)>;

    AutoTiledSurfaceBase(const TileCreator tileCreator,
                         const Request requestStart,
                         const Request requestEnd);
    AutoTiledSurfaceBase(const AutoTiledSurfaceBase& other);
    AutoTiledSurfaceBase(AutoTiledSurfaceBase&& other);

    AutoTiledSurfaceBase& operator=(const AutoTiledSurfaceBase& other);
    AutoTiledSurfaceBase& operator=(AutoTiledSurfaceBase&& other);

    ~AutoTiledSurfaceBase();

    void swap(AutoTiledSurfaceBase& other) {
        mAutoTilesData.swap(other.mAutoTilesData);
    }

    void setPixelClamp(const QRect& pixRect);
    void loadPixmap(const SkPixmap &src);
    void loadPixmap(const QImage &src);

    MyPaintRectangle paintPressEvent(MyPaintBrush * const brush,
                                     const QPointF& pos,
                                     const double dTime,
                                     const double pressure,
                                     const double xtilt,
                                     const double ytilt) const {
        mypaint_brush_reset(brush);
        mypaint_brush_new_stroke(brush);

        mypaint_surface_begin_atomic(mMyPaintSurface);
        mypaint_brush_stroke_to(brush, mMyPaintSurface,
                                pos.x(), pos.y(), pressure,
                                xtilt, ytilt, dTime);
        MyPaintRectangle roi;
        mypaint_surface_end_atomic(mMyPaintSurface, &roi);
        return roi;
    }

    MyPaintRectangle paintMoveEvent(MyPaintBrush * const brush,
                                    const QPointF &pos,
                                    const double dTime,
                                    const double pressure,
                                    const double xtilt,
                                    const double ytilt) const {
        mypaint_surface_begin_atomic(mMyPaintSurface);
        mypaint_brush_stroke_to(brush, mMyPaintSurface,
                                pos.x(), pos.y(), pressure,
                                xtilt, ytilt, dTime);
        MyPaintRectangle roi;
        mypaint_surface_end_atomic(mMyPaintSurface, &roi);
        return roi;
    }

    void execute(MyPaintBrush * const brush, const BrushStrokeSet& set) {
        set.execute(brush, mMyPaintSurface, 5);
    }

    bool tileToBitmap(const int tx, const int ty, SkBitmap& bitmap) {
        return mAutoTilesData.tileToBitmap(tx, ty, bitmap);
    }

    SkBitmap tileToBitmap(const int tx, const int ty) {
        return mAutoTilesData.tileToBitmap(tx, ty);
    }

    SkBitmap toBitmap(const QMargins& margin = QMargins()) const {
        return mAutoTilesData.toBitmap(margin);
    }

    QImage toImage(const bool use16bit,
                   const QMargins& margin = QMargins()) const {
        return mAutoTilesData.toImage(use16bit, margin);
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

    void replaceTile(const int tx, const int ty,
                     const stdsptr<Tile>& tile);

    void crop(const QRect &crop);
    void move(const int dx, const int dy);

    void discardTransparentTiles();
    void autoCrop();
protected:
    stdsptr<Tile> requestTile(const int tx, const int ty);
private:
    static void sFree(MyPaintSurface *surface);

    void free();

    MyPaintTiledSurface mParent;
    MyPaintSurface* const mMyPaintSurface;
    AutoTilesData mAutoTilesData;
    const TileCreator mTileCreator;
    const Request mRequestStart;
    const Request mRequestEnd;
};

class CORE_EXPORT AutoTiledSurface : public AutoTiledSurfaceBase {
public:
    AutoTiledSurface();

    static void sRequestStart(MyPaintTiledSurface *surface,
                              MyPaintTileRequest *request);
    static void sRequestEnd(MyPaintTiledSurface *,
                            MyPaintTileRequest *);
};
#include "undoabletile.h"
class CORE_EXPORT UndoableAutoTiledSurface : public AutoTiledSurfaceBase {
public:
    UndoableAutoTiledSurface();

    QList<UndoTile> takeUndoList() {
        for(auto& pair : mUndoList)
            pair.saveForRedoAndReset();
        const auto result = mUndoList;
        mUndoList.clear();
        return result;
    }

    void triggerAllChange();

    static void sRequestStart(MyPaintTiledSurface *surface,
                              MyPaintTileRequest *request);
    static void sRequestEnd(MyPaintTiledSurface *,
                            MyPaintTileRequest *);
private:
    void addToUndoList(const UndoTile& tile)
    { mUndoList << tile; }

    QList<UndoTile> mUndoList;
};

#endif // AUTOTILEDSURFACE_H
