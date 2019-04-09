#ifndef AUTOTILESDATA_H
#define AUTOTILESDATA_H
#include <QtCore>
#include <QList>
#include "skia/skiaincludes.h"

struct AutoTilesData {
    AutoTilesData(const int& tileSize);

    void loadBitmap(const SkBitmap& src);

    ~AutoTilesData();

    void reset();

    void stretchToTile(const int& tx, const int& ty);
    uint16_t* getTile(const int& tx, const int& ty) const;

    int width() const;
    int height() const;

    SkBitmap tileToBitmap(const int& tx, const int& ty);
    SkBitmap toBitmap(int margin = 0) const;

    bool drawOnPixmap(SkPixmap &dst, int drawX, int drawY) const;

    QPoint zeroTile() const {
        return QPoint(mZeroTileCol, mZeroTileRow);
    }

    QPoint zeroTilePos() const {
        return zeroTile()*mTileSize;
    }

    QRect pixelBoundingRect() const {
        return tileRectToPixRect(tileBoundingRect());
    }

    QRect tileBoundingRect() const {
        return QRect(-mZeroTileCol, -mZeroTileRow,
                     mColumnCount, mRowCount);
    }

    QRect tileRectToPixRect(const QRect& tileRect) const {
        return QRect(tileRect.x()*mTileSize,
                     tileRect.y()*mTileSize,
                     tileRect.width()*mTileSize,
                     tileRect.height()*mTileSize);
    }

    QRect pixRectToTileRect(const QRect& pixRect) const {
        const int widthRem = pixRect.width() % mTileSize ? 1 : 0;
        const int heightRem = pixRect.height() % mTileSize ? 1 : 0;
        return QRect(pixRect.x()/mTileSize,
                     pixRect.y()/mTileSize,
                     pixRect.width()/mTileSize + widthRem,
                     pixRect.height()/mTileSize + heightRem);
    }
private:
    uint16_t* getTileByIndex(const int& colId, const int& rowId) const;

    QList<uint16_t*> newColumn();
    void prependRows(const int& count);
    void appendRows(const int& count);
    void prependColumns(const int& count);
    void appendColumns(const int& count);

    int mZeroTileCol = 0;
    int mZeroTileRow = 0;
    int mColumnCount = 0;
    int mRowCount = 0;

    const int mTileSize;
    const size_t mTilePixSize;
    QList<QList<uint16_t*>> mColumns;
};

#endif // AUTOTILESDATA_H
