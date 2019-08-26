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

#ifndef DRAWABLEAUTOTILEDSURFACE_H
#define DRAWABLEAUTOTILEDSURFACE_H
#include "autotiledsurface.h"
#include "skia/skiahelpers.h"
#include "CacheHandlers/hddcachablecont.h"

struct TileBitmaps {
    int fRowCount = 0;
    int fColumnCount = 0;
    int fZeroTileRow = 0;
    int fZeroTileCol = 0;
    QList<QList<SkBitmap>> fBitmaps;

    TileBitmaps() {}

    TileBitmaps(const TileBitmaps& src) {
        fRowCount = src.fRowCount;
        fColumnCount = src.fColumnCount;
        fZeroTileRow = src.fZeroTileRow;
        fZeroTileCol = src.fZeroTileCol;
        fBitmaps.clear();
        for(const auto& srcList : src.fBitmaps) {
            fBitmaps << QList<SkBitmap>();
            auto& list = fBitmaps.last();
            for(const auto& srcBitmap : srcList) {
                list << SkiaHelpers::makeCopy(srcBitmap);
            }
        }
    }

    TileBitmaps(TileBitmaps &&other) {
        swap(other);
    }

    TileBitmaps &operator=(const TileBitmaps& other) {
        TileBitmaps tmp(other);
        swap(tmp);
        return *this;
    }

    TileBitmaps &operator=(TileBitmaps &&other) {
        TileBitmaps tmp(std::move(other));
        swap(tmp);
        return *this;
    }

    void write(eWriteStream& dst) const {
        dst << fRowCount;
        dst << fColumnCount;
        dst << fZeroTileRow;
        dst << fZeroTileCol;
        for(const auto& col : fBitmaps) {
            for(const auto& tile : col) {
                SkiaHelpers::writeBitmap(tile, dst);
            }
        }
    }

    void read(eReadStream& src) {
        src >> fRowCount;
        src >> fColumnCount;
        src >> fZeroTileRow;
        src >> fZeroTileCol;
        for(int i = 0; i < fColumnCount; i++) {
            fBitmaps.append(QList<SkBitmap>());
            auto& col = fBitmaps.last();
            for(int j = 0; j < fZeroTileRow; j++)
                col.append(SkiaHelpers::readBitmap(src));
        }
    }

    void swap(TileBitmaps& other) {
        fBitmaps.swap(other.fBitmaps);
        std::swap(fRowCount, other.fRowCount);
        std::swap(fColumnCount, other.fColumnCount);
        std::swap(fZeroTileRow, other.fZeroTileRow);
        std::swap(fZeroTileCol, other.fZeroTileCol);
    }

    bool isEmpty() const { return fBitmaps.isEmpty(); }

    void clear() {
        fBitmaps.clear();
        fZeroTileCol = 0;
        fZeroTileRow = 0;
        fColumnCount = 0;
        fRowCount = 0;
    }
};

class DrawableAutoTiledSurface : public HddCachable {
    e_OBJECT
    typedef QList<QList<SkBitmap>> Tiles;
public:
    DrawableAutoTiledSurface();
    DrawableAutoTiledSurface(const DrawableAutoTiledSurface& other);
    DrawableAutoTiledSurface& operator=(const DrawableAutoTiledSurface& other);
protected:
    stdsptr<eHddTask> createTmpFileDataSaver();
    stdsptr<eHddTask> createTmpFileDataLoader();

    int getByteCount() {
        const int spixels = mColumnCount*mRowCount*TILE_SPIXEL_SIZE;
        return spixels*static_cast<int>(sizeof(uint16_t));
    }

    int clearMemory() {
        const int bytes = DrawableAutoTiledSurface::getByteCount();
        clearBitmaps();
        scheduleSaveToTmpFile();
        return bytes;
    }

    void noDataLeft_k() { Q_ASSERT(false); }
public:
    void drawOnCanvas(SkCanvas * const canvas,
                      const SkPoint &dst,
                      const QRect * const minPixSrc,
                      SkPaint * const paint) const;

    void drawOnCanvas(SkCanvas * const canvas,
                      const SkPoint &dst,
                      const QRect * const minPixSrc) const {
        drawOnCanvas(canvas, dst, minPixSrc, nullptr);
    }

    void drawOnCanvas(SkCanvas * const canvas,
                      const SkPoint &dst) const {
        drawOnCanvas(canvas, dst, nullptr, nullptr);
    }

    void drawOnCanvas(SkCanvas * const canvas,
                      const SkPoint &dst,
                      SkPaint * const paint) const {
        drawOnCanvas(canvas, dst, nullptr, paint);
    }

    const AutoTiledSurface &surface() const {
        return mSurface;
    }

    void pixelRectChanged(const QRect& pixRect) {
        if(mTmpFile) scheduleDeleteTmpFile();
        updateTileRecBitmaps(pixRectToTileRect(pixRect));
    }

    QRect pixelBoundingRect() const {
        return tileRectToPixRect(tileBoundingRect());
    }

    int width() const {
        return pixelBoundingRect().width();
    }

    int height() const {
        return pixelBoundingRect().height();
    }

    void write(eWriteStream& dst) {
        if(!storesDataInMemory()) {
            if(!mTmpFile) RuntimeThrow("No tmp file, and no data in memory");
            dst.writeFile(mTmpFile.get());
        } else mSurface.write(dst);
    }

    void read(eReadStream& src) {
        mSurface.read(src);
        afterDataReplaced();
        updateTileBitmaps();
    }

    void updateTileBitmaps() {
        updateTileRecBitmaps(mSurface.tileBoundingRect());
    }

    void clearBitmaps() {
        mTileBitmaps.clear();
    }

    bool hasTileBitmaps() {
        return !mTileBitmaps.isEmpty();
    }

    void drawingDoneForNow() { afterDataReplaced(); }
private:
    void updateTileRecBitmaps(QRect tileRect);

    void setTileBitmaps(const TileBitmaps& tiles) {
        mTileBitmaps = tiles;
    }

    void setTileBitmaps(TileBitmaps&& tiles) {
        mTileBitmaps = std::move(tiles);
    }

    void stretchBitmapsToTile(const int tx, const int ty) {
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

    QList<SkBitmap> newBitmapColumn() {
        QList<SkBitmap> col;
        for(int j = 0; j < mRowCount; j++) col.append(SkBitmap());
        return col;
    }

    void prependBitmapRows(const int count) {
        for(auto& col : mBitmaps) {
            for(int i = 0; i < count; i++) {
                col.prepend(SkBitmap());
            }
        }
        mRowCount += count;
        mZeroTileRow += count;
    }

    void appendBitmapRows(const int count) {
        for(auto& col : mBitmaps) {
            for(int i = 0; i < count; i++) {
                col.append(SkBitmap());
            }
        }
        mRowCount += count;
    }

    void prependBitmapColumns(const int count) {
        for(int i = 0; i < count; i++) {
            mBitmaps.prepend(newBitmapColumn());
        }
        mColumnCount += count;
        mZeroTileCol += count;
    }

    void appendBitmapColumns(const int count) {
        for(int i = 0; i < count; i++) {
            mBitmaps.append(newBitmapColumn());
        }
        mColumnCount += count;
    }

    SkBitmap bitmapForTile(const int tx, const int ty) const {
        const auto zeroTileV = zeroTile();
        return imageForTileId(tx + zeroTileV.x(), ty + zeroTileV.y());
    }

    SkBitmap imageForTileId(const int colId, const int rowId) const {
        return mBitmaps.at(colId).at(rowId);
    }

    QPoint zeroTile() const {
        return QPoint(mZeroTileCol, mZeroTileRow);
    }

    QPoint zeroTilePos() const {
        return zeroTile()*TILE_SIZE;
    }

    QRect tileBoundingRect() const {
        return QRect(-mZeroTileCol, -mZeroTileRow,
                     mColumnCount, mRowCount);
    }

    QRect tileRectToPixRect(const QRect& tileRect) const {
        return QRect(tileRect.x()*TILE_SIZE,
                     tileRect.y()*TILE_SIZE,
                     tileRect.width()*TILE_SIZE,
                     tileRect.height()*TILE_SIZE);
    }

    QRect pixRectToTileRect(const QRect& pixRect) const {
        const int widthRem = (pixRect.width() % TILE_SIZE) ? 2 : 1;
        const int heightRem = (pixRect.height() % TILE_SIZE) ? 2 : 1;
        return QRect(qFloor(static_cast<qreal>(pixRect.x())/TILE_SIZE),
                     qFloor(static_cast<qreal>(pixRect.y())/TILE_SIZE),
                     pixRect.width()/TILE_SIZE + widthRem,
                     pixRect.height()/TILE_SIZE + heightRem);
    }

    AutoTiledSurface mSurface;
    TileBitmaps mTileBitmaps;
    int &mRowCount;
    int &mColumnCount;
    int &mZeroTileRow;
    int &mZeroTileCol;
    Tiles &mBitmaps;
};

#endif // DRAWABLEAUTOTILEDSURFACE_H
