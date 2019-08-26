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
        const DrawableAutoTiledSurface &other) : DrawableAutoTiledSurface() {
    mSurface = other.mSurface;
    mTileBitmaps = other.mTileBitmaps;
}

DrawableAutoTiledSurface &DrawableAutoTiledSurface::operator=(const DrawableAutoTiledSurface &other) {
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

#include "CacheHandlers/tmploader.h"
#include "CacheHandlers/tmpsaver.h"

class SurfaceSaver : public TmpSaver {
    e_OBJECT
public:
    typedef std::function<void(const qsptr<QTemporaryFile>&)> Func;
protected:
    SurfaceSaver(DrawableAutoTiledSurface* const target,
                 const AutoTiledSurface &surface) :
        TmpSaver(target), mSurface(surface) {}
    SurfaceSaver(DrawableAutoTiledSurface* const target,
                 AutoTiledSurface &&bitmaps) :
        TmpSaver(target), mSurface(std::move(bitmaps)) {}


    void write(eWriteStream& dst) {
        mSurface.write(dst);
    }
private:
    const AutoTiledSurface mSurface;
};

class SurfaceLoader : public TmpLoader {
    e_OBJECT
public:
    typedef std::function<void(AutoTiledSurface&&)> Func;
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
    AutoTiledSurface mSurface;
    const Func mFinishedFunc;
};

stdsptr<eHddTask> DrawableAutoTiledSurface::createTmpFileDataSaver() {
    return enve::make_shared<SurfaceSaver>(this, std::move(mSurface));
}

stdsptr<eHddTask> DrawableAutoTiledSurface::createTmpFileDataLoader() {
    stdptr<DrawableAutoTiledSurface> thisP = this;
    const SurfaceLoader::Func func =
    [thisP](AutoTiledSurface&& surface) {
        if(thisP) {
            thisP->mSurface = std::move(surface);
            thisP->updateTileBitmaps();
            thisP->afterDataLoadedFromTmpFile();
        }
    };
    return enve::make_shared<SurfaceLoader>(mTmpFile, this, func);
}
