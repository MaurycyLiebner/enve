#ifndef AUTOTILESDATA_H
#define AUTOTILESDATA_H
#include <QtCore>
#include <QList>
#include "skia/skiaincludes.h"
#include "basicreadwrite.h"
#ifndef TILE_SIZE
    #define TILE_SIZE 64
#endif

#ifndef TILE_SPIXEL_SIZE
    #define TILE_SPIXEL_SIZE 16384 //TILE_SIZE*TILE_SIZE*4
#endif

struct AutoTilesData {
    AutoTilesData();
    ~AutoTilesData();

    AutoTilesData(const AutoTilesData& other) = delete;
    AutoTilesData& operator=(const AutoTilesData& other) = delete;

    void loadBitmap(const SkBitmap& src);

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
        return zeroTile()*TILE_SIZE;
    }

    QRect pixelBoundingRect() const {
        return tileRectToPixRect(tileBoundingRect());
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
        const int widthRem = pixRect.width() % TILE_SIZE ? 1 : 0;
        const int heightRem = pixRect.height() % TILE_SIZE ? 1 : 0;
        return QRect(pixRect.x()/TILE_SIZE,
                     pixRect.y()/TILE_SIZE,
                     pixRect.width()/TILE_SIZE + widthRem,
                     pixRect.height()/TILE_SIZE + heightRem);
    }

    bool isEmpty() const { return mColumnCount == 0 || mRowCount == 0; }

    void deepCopy(const AutoTilesData &other);
    void swap(AutoTilesData& other) {
        mColumns.swap(other.mColumns);
        std::swap(mZeroTileCol, other.mZeroTileCol);
        std::swap(mZeroTileRow, other.mZeroTileRow);
        std::swap(mColumnCount, other.mColumnCount);
        std::swap(mRowCount, other.mRowCount);
    }

    void write(QIODevice * const dst) const {
        dst->write(rcConstChar(&mZeroTileCol), sizeof(int));
        dst->write(rcConstChar(&mZeroTileRow), sizeof(int));
        dst->write(rcConstChar(&mColumnCount), sizeof(int));
        dst->write(rcConstChar(&mRowCount), sizeof(int));
        const int nCols = mColumns.count();
        dst->write(rcConstChar(&nCols), sizeof(int));
        const int nRows = mColumns.isEmpty() ? 0 : mColumns.first().count();
        dst->write(rcConstChar(&nRows), sizeof(int));
        for(const auto& col : mColumns) {
            for(const auto& row : col) {
                dst->write(rcConstChar(row),
                           TILE_SPIXEL_SIZE*sizeof(uint16_t));
            }
        }
    }

    void read(QIODevice * const src);
protected:
    uint16_t* getTileByIndex(const int& colId, const int& rowId) const;
private:
    QList<uint16_t*> newColumn();
    void prependRows(const int& count);
    void appendRows(const int& count);
    void prependColumns(const int& count);
    void appendColumns(const int& count);

    int mZeroTileCol = 0;
    int mZeroTileRow = 0;
    int mColumnCount = 0;
    int mRowCount = 0;
    QList<QList<uint16_t*>> mColumns;
};

#endif // AUTOTILESDATA_H
