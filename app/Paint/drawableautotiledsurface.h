#ifndef DRAWABLEAUTOTILEDSURFACE_H
#define DRAWABLEAUTOTILEDSURFACE_H
#include "autotiledsurface.h"

class DrawableAutoTiledSurface : public AutoTiledSurface {
public:
    DrawableAutoTiledSurface();

    void drawOnCanvas(SkCanvas * const canvas,
                       const QRect &minSrc,
                       const QPoint &dst,
                       SkPaint * const paint = nullptr) const;

    void updateTileImg(const int& tx, const int& ty) {
        auto btmp = mAutoTilesData.tileToBitmap(tx, ty);
        btmp.setImmutable();
        const auto img = SkImage::MakeFromBitmap(btmp);
        btmp.reset();

        const auto zeroTile = mAutoTilesData.zeroTile();
        const auto tileId = QPoint(tx, ty) + zeroTile;
        mTileImgs[tileId.x()].replace(tileId.y(), img);
    }

    void updateTileRectImgs(const QRect& tileRect) {
        for(int tx = tileRect.left(); tx <= tileRect.right(); tx++) {
            for(int ty = tileRect.top(); ty <= tileRect.bottom(); ty++) {
                updateTileImg(tx, ty);
            }
        }
    }

    void updatePixelRectImgs(const QRect& pixRect) {
        updateTileRectImgs(mAutoTilesData.pixRectToTileRect(pixRect));
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
private:
    QList<sk_sp<SkImage>> newImgColumn() {
        QList<sk_sp<SkImage>> col;
        for(int j = 0; j < mRowCount; j++) {
            col.append(sk_sp<SkImage>());
        }
        return col;
    }

    void prependImgRows(const int &count) {
        for(auto& col : mTileImgs) {
            for(int i = 0; i < count; i++) {
                col.prepend(sk_sp<SkImage>());
            }
        }
        mRowCount += count;
        mZeroTileRow += count;
    }

    void appendImgRows(const int &count) {
        for(auto& col : mTileImgs) {
            for(int i = 0; i < count; i++) {
                col.append(sk_sp<SkImage>());
            }
        }
        mRowCount += count;
    }

    void prependImgColumns(const int &count) {
        for(int i = 0; i < count; i++) {
            mTileImgs.prepend(newImgColumn());
        }
        mColumnCount += count;
        mZeroTileCol += count;
    }

    void appendImgColumns(const int &count) {
        for(int i = 0; i < count; i++) {
            mTileImgs.append(newImgColumn());
        }
        mColumnCount += count;
    }

    SkImage * imageForTile(const int &tx, const int &ty) const {
        const auto zeroTile = mAutoTilesData.zeroTile();
        return imageForTileId(tx + zeroTile.x(), ty + zeroTile.y());
    }

    SkImage * imageForTileId(const int &colId,
                             const int &rowId) const {
        return mTileImgs.at(colId).at(rowId).get();
    }

    int mRowCount;
    int mColumnCount;
    int mZeroTileRow;
    int mZeroTileCol;
    QList<QList<sk_sp<SkImage>>> mTileImgs;
};

#endif // DRAWABLEAUTOTILEDSURFACE_H
