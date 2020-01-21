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

#include "drawableautotiledsurface.h"
#include "skia/skiahelpers.h"

DrawableAutoTiledSurface::DrawableAutoTiledSurface() :
    mRowCount(mTileBitmaps.fRowCount),
    mColumnCount(mTileBitmaps.fColumnCount),
    mZeroTileRow(mTileBitmaps.fZeroTileRow),
    mZeroTileCol(mTileBitmaps.fZeroTileCol),
    mBitmaps(mTileBitmaps.fBitmaps) {
    afterDataReplaced();
}

DrawableAutoTiledSurface::DrawableAutoTiledSurface(
        const DrawableAutoTiledSurface &other) :
    DrawableAutoTiledSurface() {
    mSurface = other.mSurface;
    mTileBitmaps = other.mTileBitmaps;
}

DrawableAutoTiledSurface &DrawableAutoTiledSurface::operator=(
        const DrawableAutoTiledSurface &other) {
    mSurface = other.mSurface;
    mTileBitmaps = other.mTileBitmaps;
    return *this;
}

void DrawableAutoTiledSurface::drawOnCanvas(SkCanvas * const canvas,
                                            const SkPoint &dst,
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
        const float drawX = dst.x() + tx*TILE_SIZE;
        for(int ty = tileRect.top(); ty <= tileRect.bottom(); ty++) {
            const auto btmp = bitmapForTile(tx, ty);
            if(btmp.isNull()) continue;
            const float drawY = dst.y() + ty*TILE_SIZE;
            canvas->drawBitmap(btmp, drawX, drawY, paint);
        }
    }
}

void DrawableAutoTiledSurface::pixelRectChanged(const QRect &pixRect) {
    if(mTmpFile) scheduleDeleteTmpFile();
    updateTileRecBitmaps(pixRectToTileRect(pixRect));
}

void DrawableAutoTiledSurface::write(eWriteStream &dst) {
    if(!storesDataInMemory()) {
        if(!mTmpFile) RuntimeThrow("No tmp file, and no data in memory");
        dst.writeFile(mTmpFile.get());
    } else mSurface.write(dst);
}

void DrawableAutoTiledSurface::read(eReadStream &src) {
    mSurface.read(src);
    afterDataReplaced();
    updateTileBitmaps();
}

void DrawableAutoTiledSurface::updateTileBitmaps() {
    updateTileRecBitmaps(mSurface.tileBoundingRect());
}

void DrawableAutoTiledSurface::clearBitmaps() {
    mTileBitmaps.clear();
}

void DrawableAutoTiledSurface::autoCrop() {
    mSurface.autoCrop();
    // 2020.01.22 TODO remove only unnecessary bitmaps
    clearBitmaps();
    updateTileBitmaps();
}

void DrawableAutoTiledSurface::updateTileRecBitmaps(QRect tileRect) {
    const QRect maxRect = mSurface.tileBoundingRect();
    if(!maxRect.intersects(tileRect)) return;
    tileRect = maxRect.intersected(tileRect);
    const auto min = tileRect.topLeft();
    const auto max = tileRect.bottomRight();
    stretchBitmapsToTile(min.x(), min.y());
    stretchBitmapsToTile(max.x(), max.y());
    const int n = tileRect.width()*tileRect.height();
    #pragma omp parallel for collapse(2) if(n > 4)
    for(int tx = tileRect.left(); tx <= tileRect.right(); tx++) {
        for(int ty = tileRect.top(); ty <= tileRect.bottom(); ty++) {
            const auto tileId = QPoint(tx, ty) + zeroTile();
            SkBitmap& btmp = mBitmaps[tileId.x()][tileId.y()];
            if(btmp.isNull()) {
                btmp = mSurface.tileToBitmap(tx, ty);
            } else {
                mSurface.tileToBitmap(tx, ty, btmp);
                btmp.notifyPixelsChanged();
            }
        }
    }
}

void DrawableAutoTiledSurface::setTileBitmaps(const TileBitmaps &tiles) {
    mTileBitmaps = tiles;
}

void DrawableAutoTiledSurface::setTileBitmaps(TileBitmaps &&tiles) {
    mTileBitmaps = std::move(tiles);
}

void DrawableAutoTiledSurface::stretchBitmapsToTile(const int tx, const int ty) {
    const int colId = tx + mZeroTileCol;
    const int rowId = ty + mZeroTileRow;

    if(rowId < 0) {
        prependBitmapRows(qAbs(rowId));
    } else if(rowId >= mRowCount) {
        appendBitmapRows(qAbs(rowId - mRowCount + 1));
    }
    if(colId < 0) {
        prependBitmapColumns(qAbs(colId));
    } else if(colId >= mColumnCount) {
        appendBitmapColumns(qAbs(colId - mColumnCount + 1));
    }
}

QList<SkBitmap> DrawableAutoTiledSurface::newBitmapColumn() {
    QList<SkBitmap> col;
    for(int j = 0; j < mRowCount; j++) col.append(SkBitmap());
    return col;
}

void DrawableAutoTiledSurface::prependBitmapRows(const int count) {
    for(auto& col : mBitmaps) {
        for(int i = 0; i < count; i++) {
            col.prepend(SkBitmap());
        }
    }
    mRowCount += count;
    mZeroTileRow += count;
}

void DrawableAutoTiledSurface::appendBitmapRows(const int count) {
    for(auto& col : mBitmaps) {
        for(int i = 0; i < count; i++) {
            col.append(SkBitmap());
        }
    }
    mRowCount += count;
}

void DrawableAutoTiledSurface::prependBitmapColumns(const int count) {
    for(int i = 0; i < count; i++) {
        mBitmaps.prepend(newBitmapColumn());
    }
    mColumnCount += count;
    mZeroTileCol += count;
}

void DrawableAutoTiledSurface::appendBitmapColumns(const int count) {
    for(int i = 0; i < count; i++) {
        mBitmaps.append(newBitmapColumn());
    }
    mColumnCount += count;
}

SkBitmap DrawableAutoTiledSurface::bitmapForTile(const int tx, const int ty) const {
    const auto zeroTileV = zeroTile();
    return imageForTileId(tx + zeroTileV.x(), ty + zeroTileV.y());
}

SkBitmap DrawableAutoTiledSurface::imageForTileId(const int colId, const int rowId) const {
    return mBitmaps.at(colId).at(rowId);
}

QRect DrawableAutoTiledSurface::tileBoundingRect() const {
    return QRect(-mZeroTileCol, -mZeroTileRow, mColumnCount, mRowCount);
}

QRect DrawableAutoTiledSurface::tileRectToPixRect(const QRect &tileRect) const {
    return QRect(tileRect.x()*TILE_SIZE,
                 tileRect.y()*TILE_SIZE,
                 tileRect.width()*TILE_SIZE,
                 tileRect.height()*TILE_SIZE);
}

QRect DrawableAutoTiledSurface::pixRectToTileRect(const QRect &pixRect) const {
    const int widthRem = (pixRect.width() % TILE_SIZE) ? 2 : 1;
    const int heightRem = (pixRect.height() % TILE_SIZE) ? 2 : 1;
    return QRect(qFloor(static_cast<qreal>(pixRect.x())/TILE_SIZE),
                 qFloor(static_cast<qreal>(pixRect.y())/TILE_SIZE),
                 pixRect.width()/TILE_SIZE + widthRem,
                 pixRect.height()/TILE_SIZE + heightRem);
}

#include "CacheHandlers/tmploader.h"
#include "CacheHandlers/tmpsaver.h"

class SurfaceSaver : public TmpSaver {
    e_OBJECT
    public:
        typedef std::function<void(const qsptr<QTemporaryFile>&)> Func;
protected:
    SurfaceSaver(DrawableAutoTiledSurface* const target,
                 const UndoableAutoTiledSurface &surface) :
        TmpSaver(target), mSurface(surface) {}
    SurfaceSaver(DrawableAutoTiledSurface* const target,
                 UndoableAutoTiledSurface &&bitmaps) :
        TmpSaver(target), mSurface(std::move(bitmaps)) {}


    void write(eWriteStream& dst) {
        mSurface.write(dst);
    }
private:
    const UndoableAutoTiledSurface mSurface;
};

class SurfaceLoader : public TmpLoader {
    e_OBJECT
public:
    typedef std::function<void(UndoableAutoTiledSurface&&)> Func;
protected:
    SurfaceLoader(const qsptr<QTemporaryFile> &file,
                  DrawableAutoTiledSurface* const target,
                  const Func& finishedFunc) :
        TmpLoader(file, target),
        mFinishedFunc(finishedFunc) {}

    void read(eReadStream& src) {
        mSurface.read(src);
    }
    void afterProcessing() {
        if(mFinishedFunc) mFinishedFunc(std::move(mSurface));
    }
private:
    UndoableAutoTiledSurface mSurface;
    const Func mFinishedFunc;
};

stdsptr<eHddTask> DrawableAutoTiledSurface::createTmpFileDataSaver() {
    return enve::make_shared<SurfaceSaver>(this, std::move(mSurface));
}

stdsptr<eHddTask> DrawableAutoTiledSurface::createTmpFileDataLoader() {
    stdptr<DrawableAutoTiledSurface> thisP = this;
    const SurfaceLoader::Func func =
    [thisP](UndoableAutoTiledSurface&& surface) {
        if(thisP) {
            thisP->mSurface = std::move(surface);
            thisP->updateTileBitmaps();
            thisP->afterDataLoadedFromTmpFile();
        }
    };
    return enve::make_shared<SurfaceLoader>(mTmpFile, this, func);
}

int DrawableAutoTiledSurface::getByteCount() {
    const int spixels = mColumnCount*mRowCount*TILE_SPIXEL_SIZE;
    return spixels*static_cast<int>(sizeof(uint16_t));
}

int DrawableAutoTiledSurface::clearMemory() {
    const int bytes = DrawableAutoTiledSurface::getByteCount();
    clearBitmaps();
    scheduleSaveToTmpFile();
    return bytes;
}
