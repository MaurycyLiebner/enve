#ifndef DRAWABLEAUTOTILEDSURFACE_H
#define DRAWABLEAUTOTILEDSURFACE_H
#include "autotiledsurface.h"

class DrawableAutoTiledSurface {
public:
    DrawableAutoTiledSurface();

    void drawOnCanvas(SkCanvas * const canvas,
                      const QRect &minPixSrc,
                      const QPoint &dst,
                      SkPaint * const paint = nullptr) const;

    AutoTiledSurface * getTarget() const {
        return mTarget;
    }

    void updateTileImages() {
        if(mTarget) updateTileRectImgs(mTarget->tileBoundingRect());
        else mTileImgs.clear();
    }

    void updateTileRectImgs(QRect tileRect) {
        if(!mTarget) return;
        const QRect maxRect = mTarget->tileBoundingRect();
        if(!maxRect.intersects(tileRect)) return;
        tileRect = maxRect.intersected(tileRect);
        const auto min = tileRect.topLeft();
        const auto max = tileRect.bottomRight();
        stretchToTileImg(min.x(), min.y());
        stretchToTileImg(max.x(), max.y());
        for(int tx = tileRect.left(); tx <= tileRect.right(); tx++) {
            for(int ty = tileRect.top(); ty <= tileRect.bottom(); ty++) {
                auto btmp = mTarget->tileToBitmap(tx, ty);
                btmp.setImmutable();
                const auto img = SkImage::MakeFromBitmap(btmp);
                btmp.reset();

                const auto tileId = QPoint(tx, ty) + zeroTile();
                mTileImgs[tileId.x()].replace(tileId.y(), img);
            }
        }
    }

    void updatePixelRectImgs(const QRect& pixRect) {
        updateTileRectImgs(pixRectToTileRect(pixRect));
    }

    void setTarget(AutoTiledSurface * const target) {
        if(mTarget == target) return;
        mTarget = target;
        updateTileImages();
    }

    QRect pixelBoundingRect() const {
        return tileRectToPixRect(tileBoundingRect());
    }
private:
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
        const auto zeroTileV = zeroTile();
        return imageForTileId(tx + zeroTileV.x(), ty + zeroTileV.y());
    }

    SkImage * imageForTileId(const int &colId,
                             const int &rowId) const {
        return mTileImgs.at(colId).at(rowId).get();
    }

    QPoint zeroTile() const {
        return QPoint(mZeroTileCol, mZeroTileRow);
    }

    QPoint zeroTilePos() const {
        return zeroTile()*MYPAINT_TILE_SIZE;
    }

    QRect tileBoundingRect() const {
        return QRect(-mZeroTileCol, -mZeroTileRow,
                     mColumnCount, mRowCount);
    }

    QRect tileRectToPixRect(const QRect& tileRect) const {
        return QRect(tileRect.x()*MYPAINT_TILE_SIZE,
                     tileRect.y()*MYPAINT_TILE_SIZE,
                     tileRect.width()*MYPAINT_TILE_SIZE,
                     tileRect.height()*MYPAINT_TILE_SIZE);
    }

    QRect pixRectToTileRect(const QRect& pixRect) const {
        const int widthRem = pixRect.width() % MYPAINT_TILE_SIZE ? 1 : 0;
        const int heightRem = pixRect.height() % MYPAINT_TILE_SIZE ? 1 : 0;
        return QRect(pixRect.x()/MYPAINT_TILE_SIZE,
                     pixRect.y()/MYPAINT_TILE_SIZE,
                     pixRect.width()/MYPAINT_TILE_SIZE + widthRem,
                     pixRect.height()/MYPAINT_TILE_SIZE + heightRem);
    }

    AutoTiledSurface * mTarget = nullptr;
    int mRowCount;
    int mColumnCount;
    int mZeroTileRow;
    int mZeroTileCol;
    QList<QList<sk_sp<SkImage>>> mTileImgs;
};

#endif // DRAWABLEAUTOTILEDSURFACE_H
