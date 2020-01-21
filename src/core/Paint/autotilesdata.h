// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#ifndef AUTOTILESDATA_H
#define AUTOTILESDATA_H
#include <QtCore>
#include <QList>
#include "skia/skiaincludes.h"
#include "../ReadWrite/basicreadwrite.h"
#include "glhelpers.h"
#include "tile.h"
#include "smartPointers/stdselfref.h"

#ifndef TILE_SIZE
    #define TILE_SIZE 64
#endif

#ifndef TILE_SPIXEL_SIZE
    #define TILE_SPIXEL_SIZE TILE_SIZE*TILE_SIZE*4
#endif

struct AutoTilesData {
    using TileCreator = std::function<stdsptr<Tile>(const size_t&)>;

    AutoTilesData(const TileCreator& tileCreator);
    AutoTilesData(const AutoTilesData& other);
    AutoTilesData(AutoTilesData&& other);

    AutoTilesData& operator=(const AutoTilesData& other);
    AutoTilesData& operator=(AutoTilesData&& other);

    ~AutoTilesData();

    void loadBitmap(const SkBitmap& src);

    void clear();

    stdsptr<Tile> requestTile(const int tx, const int ty);
    bool stretchToTile(const int tx, const int ty);
    void replaceTile(const int tx, const int ty,
                     const stdsptr<Tile>& tile);
    stdsptr<Tile> getTile(const int tx, const int ty) const;

    int width() const;
    int height() const;

    bool tileToBitmap(const int tx, const int ty, SkBitmap &bitmap);
    bool tileToBitmap(const Tile &srcTile, SkBitmap& bitmap);
    SkBitmap tileToBitmap(const int tx, const int ty);
    SkBitmap toBitmap(const QMargins& margin = QMargins()) const;

    bool drawOnPixmap(SkPixmap &dst, int drawX, int drawY) const;

    QPoint zeroTile() const;
    QPoint zeroTilePos() const;
    QRect pixelBoundingRect() const;
    QRect tileBoundingRect() const;
    QRect tileRectToPixRect(const QRect& tileRect) const;
    QRect pixRectToTileRect(const QRect& pixRect) const;

    void setPixelClamp(const QRect& pixRect);

    bool isEmpty() const { return mColumnCount == 0 || mRowCount == 0; }

    void swap(AutoTilesData& other);

    void write(eWriteStream &dst) const;
    void read(eReadStream& src);

    void discardTransparentTiles();
    void autoCrop();
protected:
    stdsptr<Tile> getTileByIndex(const int colId, const int rowId) const;
private:
    bool columnEmpty(const int colId);
    bool rowEmpty(const int rowId);

    bool cropFirstColumnIfEmpty();
    bool cropLastColumnIfEmpty();
    bool cropFirstRowIfEmpty();
    bool cropLastRowIfEmpty();

    QList<stdsptr<Tile>> newColumn();
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
    QList<QList<stdsptr<Tile>>> mColumns;

    const TileCreator mTileCreator;
};

#endif // AUTOTILESDATA_H
