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
#include "glhelpers.h"

struct AutoTilesData {
    AutoTilesData();
    ~AutoTilesData();

    AutoTilesData(const AutoTilesData& other) = delete;
    AutoTilesData& operator=(const AutoTilesData& other) = delete;

    void loadBitmap(const SkBitmap& src);

    void reset();

    bool stretchToTile(const int tx, const int ty);
    uint16_t* getTile(const int tx, const int ty) const;

    int width() const;
    int height() const;

    void tileToBitmap(const int tx, const int ty, SkBitmap& bitmap);
    SkBitmap tileToBitmap(const int tx, const int ty);
    SkBitmap toBitmap(const QMargins& margin = QMargins()) const;

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

    void setPixelClamp(const QRect& pixRect) {
        mMinCol = qFloor(qreal(pixRect.left())/TILE_SIZE);
        mMaxCol = qCeil(qreal(pixRect.right())/TILE_SIZE);
        mMinRow = qFloor(qreal(pixRect.top())/TILE_SIZE);
        mMaxRow = qCeil(qreal(pixRect.bottom())/TILE_SIZE);
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

    void write(QIODevice * const dst) const;
    void read(QIODevice * const src);
protected:
    uint16_t* getTileByIndex(const int colId, const int rowId) const;
private:
    QList<uint16_t*> newColumn();
    void prependRows(const int count);
    void appendRows(const int count);
    void prependColumns(const int count);
    void appendColumns(const int count);

    int mMinCol = -100;
    int mMaxCol = 100;
    int mMinRow = -100;
    int mMaxRow = 100;

    int mZeroTileCol = 0;
    int mZeroTileRow = 0;
    int mColumnCount = 0;
    int mRowCount = 0;
    QList<QList<uint16_t*>> mColumns;
};

#endif // AUTOTILESDATA_H
