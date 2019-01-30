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

    uint16_t* getTileRelToZero(const int& relCol, const int& relRow);

    uint16_t* cGetTile(const int& col, const int& row) const;

    int width() const;

    int height() const;

    SkBitmap toBitmap() const;

    bool drawOnPixmapZeroTilePivoted(
            SkPixmap &dst, const int& drawX, const int& drawY) const;

    bool drawOnPixmap(
            SkPixmap &dst, const int& drawX, const int& drawY) const;

    QPoint zeroTilePos() const {
        return QPoint(mZeroTileCol*mTileSize, mZeroTileRow*mTileSize);
    }
private:
    void stretchToIncludeRel(const int& relCol, const int& relRow);

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
