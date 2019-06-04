#ifndef DRAWABLEAUTOTILEDSURFACE_H
#define DRAWABLEAUTOTILEDSURFACE_H
#include "autotiledsurface.h"
#include "skia/skiahelpers.h"
#include "CacheHandlers/hddcachablecontainer.h"

struct TileImgs {
    int fRowCount = 0;
    int fColumnCount = 0;
    int fZeroTileRow = 0;
    int fZeroTileCol = 0;
    QList<QList<sk_sp<SkImage>>> fImgs;

    void deepCopy(const TileImgs& src) {
        fRowCount = src.fRowCount;
        fColumnCount = src.fColumnCount;
        fZeroTileRow = src.fZeroTileRow;
        fZeroTileCol = src.fZeroTileCol;
        fImgs.clear();
        for(const auto& srcList : src.fImgs) {
            fImgs << QList<sk_sp<SkImage>>();
            auto& list = fImgs.last();
            for(const auto& srcImg : srcList) {
                list << SkiaHelpers::makeCopy(srcImg);
            }
        }
    }

    bool isEmpty() const { return fImgs.isEmpty(); }

    void clear() {
        fImgs.clear();
        fZeroTileCol = 0;
        fZeroTileRow = 0;
        fColumnCount = 0;
        fRowCount = 0;
    }
};

class DrawableAutoTiledSurface : public HDDCachablePersistent {
    friend class StdSelfRef;
    typedef  QList<QList<sk_sp<SkImage>>> Tiles;
protected:
    DrawableAutoTiledSurface();
    DrawableAutoTiledSurface(const DrawableAutoTiledSurface& other) = delete;
    DrawableAutoTiledSurface& operator=(const DrawableAutoTiledSurface& other) = delete;

    stdsptr<HDDTask> createTmpFileDataSaver();

    stdsptr<HDDTask> createTmpFileDataLoader();

    int getByteCount() {
        const int spixels = mColumnCount*mRowCount*TILE_SPIXEL_SIZE;
        return spixels*static_cast<int>(sizeof(uint16_t));
    }

    int clearMemory() {
        const int bytes = getByteCount();
        clearImgs();
        return bytes;
    }
public:
    void deepCopy(const DrawableAutoTiledSurface& other) {
        mSurface.deepCopy(other.mSurface);
        mTileImgs.deepCopy(other.mTileImgs);
    }

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
        updateTileRectImgs(pixRectToTileRect(pixRect));
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
private:
    void updateTileImages() {
        updateTileRectImgs(mSurface.tileBoundingRect());
    }

    void updateTileRectImgs(QRect tileRect) {
        const QRect maxRect = mSurface.tileBoundingRect();
        if(!maxRect.intersects(tileRect)) return;
        tileRect = maxRect.intersected(tileRect);
        const auto min = tileRect.topLeft();
        const auto max = tileRect.bottomRight();
        stretchToTileImg(min.x(), min.y());
        stretchToTileImg(max.x(), max.y());
        for(int tx = tileRect.left(); tx <= tileRect.right(); tx++) {
            for(int ty = tileRect.top(); ty <= tileRect.bottom(); ty++) {
                auto btmp = mSurface.tileToBitmap(tx, ty);
                const auto img = SkiaHelpers::transferDataToSkImage(btmp);

                const auto tileId = QPoint(tx, ty) + zeroTile();
                mImgs[tileId.x()].replace(tileId.y(), img);
            }
        }
    }

    void setTileImgs(const TileImgs& tiles) {
        mTileImgs = tiles;
    }

    void clearImgs() {
        mTileImgs.clear();
    }

    void stretchToTileImg(const int &tx, const int &ty) {
        const int colId = tx + mZeroTileCol;
        const int rowId = ty + mZeroTileRow;

        if(rowId < 0) {
            prependImgRows(qAbs(rowId));
        } else if(rowId >= mRowCount) {
            appendImgRows(qAbs(rowId - mRowCount + 1));
        }
        if(colId < 0) {
            prependImgColumns(qAbs(colId));
        } else if(colId >= mColumnCount) {
            appendImgColumns(qAbs(colId - mColumnCount + 1));
        }
    }

    QList<sk_sp<SkImage>> newImgColumn() {
        QList<sk_sp<SkImage>> col;
        for(int j = 0; j < mRowCount; j++) {
            col.append(sk_sp<SkImage>());
        }
        return col;
    }

    void prependImgRows(const int &count) {
        for(auto& col : mImgs) {
            for(int i = 0; i < count; i++) {
                col.prepend(sk_sp<SkImage>());
            }
        }
        mRowCount += count;
        mZeroTileRow += count;
    }

    void appendImgRows(const int &count) {
        for(auto& col : mImgs) {
            for(int i = 0; i < count; i++) {
                col.append(sk_sp<SkImage>());
            }
        }
        mRowCount += count;
    }

    void prependImgColumns(const int &count) {
        for(int i = 0; i < count; i++) {
            mImgs.prepend(newImgColumn());
        }
        mColumnCount += count;
        mZeroTileCol += count;
    }

    void appendImgColumns(const int &count) {
        for(int i = 0; i < count; i++) {
            mImgs.append(newImgColumn());
        }
        mColumnCount += count;
    }

    SkImage * imageForTile(const int &tx, const int &ty) const {
        const auto zeroTileV = zeroTile();
        return imageForTileId(tx + zeroTileV.x(), ty + zeroTileV.y());
    }

    SkImage * imageForTileId(const int &colId, const int &rowId) const {
        return mImgs.at(colId).at(rowId).get();
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
    TileImgs mTileImgs;
    int &mRowCount;
    int &mColumnCount;
    int &mZeroTileRow;
    int &mZeroTileCol;
    Tiles &mImgs;
};

#endif // DRAWABLEAUTOTILEDSURFACE_H
