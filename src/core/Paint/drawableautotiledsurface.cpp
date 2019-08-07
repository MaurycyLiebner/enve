#include "drawableautotiledsurface.h"
#include "castmacros.h"
#include "skia/skiahelpers.h"

DrawableAutoTiledSurface::DrawableAutoTiledSurface() :
    mRowCount(mTileBitmaps.fRowCount),
    mColumnCount(mTileBitmaps.fColumnCount),
    mZeroTileRow(mTileBitmaps.fZeroTileRow),
    mZeroTileCol(mTileBitmaps.fZeroTileCol),
    mBitmaps(mTileBitmaps.fBitmaps) {}

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

class TilesTmpFileDataSaver : public TmpFileDataSaver {
    e_OBJECT
public:
    typedef std::function<void(const qsptr<QTemporaryFile>&)> Func;
protected:
    TilesTmpFileDataSaver(const TileBitmaps &bitmaps,
                          const Func& finishedFunc) :
        mBitmaps(bitmaps), mFinishedFunc(finishedFunc) {}

    void writeToFile(QIODevice * const file) {
        file->write(rcConstChar(&mBitmaps.fRowCount), sizeof(int));
        file->write(rcConstChar(&mBitmaps.fColumnCount), sizeof(int));
        file->write(rcConstChar(&mBitmaps.fZeroTileRow), sizeof(int));
        file->write(rcConstChar(&mBitmaps.fZeroTileCol), sizeof(int));
        for(const auto& col : mBitmaps.fBitmaps) {
            for(const auto& tile : col) {
                SkiaHelpers::writeBitmap(tile, file);
            }
        }
    }

    void afterProcessing() {
        if(mFinishedFunc) mFinishedFunc(mTmpFile);
    }
private:
    const TileBitmaps mBitmaps;
    const Func mFinishedFunc;
};

class TilesTmpFileDataLoader : public TmpFileDataLoader {
    e_OBJECT
public:
    typedef std::function<void(const TileBitmaps&)> Func;
protected:
    TilesTmpFileDataLoader(const qsptr<QTemporaryFile> &file,
                           const Func& finishedFunc) :
        TmpFileDataLoader(file), mFinishedFunc(finishedFunc) {}

    void readFromFile(QIODevice * const file) {
        file->read(rcChar(&mTileBitmaps.fRowCount), sizeof(int));
        file->read(rcChar(&mTileBitmaps.fColumnCount), sizeof(int));
        file->read(rcChar(&mTileBitmaps.fZeroTileRow), sizeof(int));
        file->read(rcChar(&mTileBitmaps.fZeroTileCol), sizeof(int));
        for(int i = 0; i < mTileBitmaps.fColumnCount; i++) {
            mTileBitmaps.fBitmaps.append(QList<SkBitmap>());
            auto& col = mTileBitmaps.fBitmaps.last();
            for(int j = 0; j < mTileBitmaps.fZeroTileRow; j++)
                col.append(SkiaHelpers::readBitmap(file));
        }
    }
    void afterProcessing() {
        if(mFinishedFunc) mFinishedFunc(mTileBitmaps);
    }
private:
    TileBitmaps mTileBitmaps;
    const Func mFinishedFunc;
};


stdsptr<HDDTask> DrawableAutoTiledSurface::createTmpFileDataSaver() {
    const TilesTmpFileDataSaver::Func func =
            [this](const qsptr<QTemporaryFile>& tmpFile) {
        setDataSavedToTmpFile(tmpFile);
    };
    return enve::make_shared<TilesTmpFileDataSaver>(mTileBitmaps, func);
}

stdsptr<HDDTask> DrawableAutoTiledSurface::createTmpFileDataLoader() {
    const TilesTmpFileDataLoader::Func func =
            [this](const TileBitmaps& tiles) {
        setTileBitmaps(tiles);
    };
    return enve::make_shared<TilesTmpFileDataLoader>(mTmpFile, func);
}
