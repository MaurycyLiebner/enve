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

#include "autotilesdata.h"
#include "exceptions.h"
#include "skia/skiahelpers.h"

AutoTilesData::AutoTilesData(const TileCreator& tileCreator) :
    mTileCreator(tileCreator) {}

AutoTilesData::AutoTilesData(const AutoTilesData &other) :
    AutoTilesData(other.mTileCreator) {
    mZeroTileCol = other.mZeroTileCol;
    mZeroTileRow = other.mZeroTileRow;
    mColumnCount = other.mColumnCount;
    mRowCount = other.mRowCount;

    for(auto& column : other.mColumns) {
        mColumns.append(QList<stdsptr<Tile>>());
        QList<stdsptr<Tile>> &col = mColumns.last();
        for(auto& srcTile : column) {
            const auto dstTile = mTileCreator(TILE_SPIXEL_SIZE);
            dstTile->copyFrom(*srcTile.get());
            col << dstTile;
        }
    }
}

AutoTilesData::AutoTilesData(AutoTilesData &&other) :
    AutoTilesData(other.mTileCreator) {
    swap(other);
}

AutoTilesData &AutoTilesData::operator=(const AutoTilesData& other) {
    AutoTilesData tmp(other);
    swap(tmp);
    return *this;
}

AutoTilesData &AutoTilesData::operator=(AutoTilesData &&other) {
    AutoTilesData tmp(std::move(other));
    swap(tmp);
    return *this;
}

void AutoTilesData::loadBitmap(const SkBitmap &src) {
    clear();
    const int nCols = qCeil(static_cast<qreal>(src.width())/TILE_SIZE);
    const int nRows = qCeil(static_cast<qreal>(src.height())/TILE_SIZE);
    const uint8_t * const srcP = static_cast<uint8_t*>(src.getPixels());
    for(int col = 0; col < nCols; col++) {
        const bool lastCol = col == (nCols - 1);
        const int x0 = col*TILE_SIZE;
        const int maxX = qMin(x0 + TILE_SIZE, src.width());
        QList<stdsptr<Tile>> colRows;
        for(int row = 0; row < nRows; row++) {
            const bool lastRow = row == (nRows - 1);
            const bool iniZeroed = lastCol || lastRow;
            const auto tile = mTileCreator(TILE_SPIXEL_SIZE);
            try {
                if(iniZeroed) tile->zeroData();
            } catch(...) {
                clear();
                RuntimeThrow("Failed to load bitmap to AutoTilesData.");
            }

            const int y0 = row*TILE_SIZE;
            const int maxY = qMin(y0 + TILE_SIZE, src.height());
            const auto tileP = tile->requestData();
            for(int y = y0; y < maxY; y++) {
                const uint8_t * srcLine = srcP + (y*src.width() + x0)*4;
                uint16_t* dstLine = tileP + (y - y0)*TILE_SIZE*4;
                for(int x = x0; x < maxX; x++) {
                    *dstLine++ = (*srcLine++ * (1<<15) + 255/2) / 255;
                    *dstLine++ = (*srcLine++ * (1<<15) + 255/2) / 255;
                    *dstLine++ = (*srcLine++ * (1<<15) + 255/2) / 255;
                    *dstLine++ = (*srcLine++ * (1<<15) + 255/2) / 255;
                }
            }
            colRows << tile;
        }
        mColumns << colRows;
    }
    mColumnCount = nCols;
    mRowCount = nRows;
    mZeroTileCol = 0;
    mZeroTileRow = 0;
}

AutoTilesData::~AutoTilesData() {
    clear();
}

void AutoTilesData::clear() {
    mColumns.clear();
    mZeroTileCol = 0;
    mZeroTileRow = 0;
    mColumnCount = 0;
    mRowCount = 0;
}

stdsptr<Tile> AutoTilesData::getTile(const int tx, const int ty) const {
    return getTileByIndex(tx + mZeroTileCol, ty + mZeroTileRow);
}

stdsptr<Tile> AutoTilesData::getTileByIndex(const int colId,
                                            const int rowId) const {
    if(colId < 0 || colId >= mColumnCount ||
       rowId < 0 || rowId >= mRowCount) return nullptr;
    return mColumns.at(colId).at(rowId);
}

int AutoTilesData::width() const {
    return mColumnCount*TILE_SIZE;
}

int AutoTilesData::height() const {
    return mRowCount*TILE_SIZE;
}

bool AutoTilesData::tileToBitmap(const Tile &srcTile, SkBitmap &bitmap) {
    Q_ASSERT(bitmap.width() == TILE_SIZE);
    Q_ASSERT(bitmap.height() == TILE_SIZE);
    const uint16_t * const srcP = srcTile.data();
    uint8_t * const dstP = static_cast<uint8_t*>(bitmap.getPixels());
    Q_ASSERT(dstP);
    if(!srcP) {
        for(int y = 0; y < TILE_SIZE; y++) {
            uint8_t * dstLine = dstP + y*bitmap.width()*4;
            for(int x = 0; x < TILE_SIZE; x++) {
                *dstLine++ = 0;
                *dstLine++ = 0;
                *dstLine++ = 0;
                *dstLine++ = 0;
            }
        }
        return false;
    }

    for(int y = 0; y < TILE_SIZE; y++) {
        uint8_t * dstLine = dstP + y*bitmap.width()*4;
        const uint16_t * srcLine = srcP + y*TILE_SIZE*4;
        for(int x = 0; x < TILE_SIZE; x++) {
            const uint32_t r = *srcLine++;
            const uint32_t g = *srcLine++;
            const uint32_t b = *srcLine++;
            const uint32_t a = *srcLine++;

            *dstLine++ = (r * 255 + (1<<15)/2) / (1<<15);
            *dstLine++ = (g * 255 + (1<<15)/2) / (1<<15);
            *dstLine++ = (b * 255 + (1<<15)/2) / (1<<15);
            *dstLine++ = (a * 255 + (1<<15)/2) / (1<<15);
        }
    }
    return true;
}

SkBitmap AutoTilesData::tileToBitmap(const int tx, const int ty) {
    SkBitmap bitmap;
    const auto srcTile = getTile(tx, ty);
    if(!srcTile->data()) return bitmap;
    const auto info = SkiaHelpers::getPremulRGBAInfo(TILE_SIZE, TILE_SIZE);
    bitmap.allocPixels(info);
    tileToBitmap(*srcTile, bitmap);
    return bitmap;
}

bool AutoTilesData::tileToBitmap(const int tx, const int ty, SkBitmap &bitmap) {
    const auto srcTile = getTile(tx, ty);
    return tileToBitmap(*srcTile, bitmap);
}

void clearRect(const QRect& rect, SkBitmap& dst) {
    uint8_t * const dstP = static_cast<uint8_t*>(dst.getPixels());

    for(int y = rect.top(); y < rect.bottom(); y++) {
        const int firstRowPixelId = y*dst.width() + rect.left();
        uint8_t * dstLine = dstP + firstRowPixelId*4;
        for(int x = rect.left(); x < rect.right(); x++) {
            *dstLine++ = 0;
            *dstLine++ = 0;
            *dstLine++ = 0;
            *dstLine++ = 0;
        }
    }
}

void clearMarginPixels(const QMargins& margin, SkBitmap& dst) {
    const int lM = qMax(0, margin.left());
    const int tM = qMax(0, margin.top());
    const int rM = qMax(0, margin.right());
    const int bM = qMax(0, margin.bottom());

    const int width = dst.width();
    const int height = dst.height();

    clearRect(QRect(0, 0, lM, height), dst);
    clearRect(QRect(width - rM, 0, rM, height), dst);
    clearRect(QRect(lM, 0, width - lM - rM, tM), dst);
    clearRect(QRect(lM, height - bM, width - lM - rM, bM), dst);
}

SkBitmap AutoTilesData::toBitmap(const QMargins& margin) const {
    const int lM = margin.left();
    const int tM = margin.top();
    const int rM = margin.right();
    const int bM = margin.bottom();

    const int dstWidth  = width()  + lM + rM;
    const int dstHeight = height() + tM + bM;
    const auto info = SkiaHelpers::getPremulRGBAInfo(dstWidth, dstHeight);
    SkBitmap dst;
    dst.allocPixels(info);

    clearMarginPixels(margin, dst);

    uint8_t * const dstP = static_cast<uint8_t*>(dst.getPixels());

    const QPoint dstTL(qMax(0, lM),
                       qMax(0, tM));
    const QPoint dstBR(dst.width()  - 1 - qMax(0, rM),
                       dst.height() - 1 - qMax(0, bM));

    const QRect dstRect(dstTL, dstBR);
    const QPoint srcTL(qMax(0, -lM),
                       qMax(0, -tM));
    const QPoint srcBR(width()  - 1 - qMax(0, -rM),
                       height() - 1 - qMax(0, -bM));
    const QRect srcRect(srcTL, srcBR);
    if(!srcRect.isValid()) return dst;
    const QPoint srcTileTL(srcRect.left()/ TILE_SIZE,
                           srcRect.top() / TILE_SIZE);
    const QPoint srcTileBR(srcRect.right() / TILE_SIZE,
                           srcRect.bottom()/ TILE_SIZE);
    const QRect srcTileRect(srcTileTL, srcTileBR);

    const int minSrcCol = srcTileRect.left();
    const int maxSrcCol = srcTileRect.right();
    const int minSrcRow = srcTileRect.top();
    const int maxSrcRow = srcTileRect.bottom();
    for(int srcCol = minSrcCol; srcCol <= maxSrcCol; srcCol++) {
        for(int srcRow = minSrcRow; srcRow <= maxSrcRow; srcRow++) {
            const QRect tSrcRect(srcCol*TILE_SIZE,
                                 srcRow*TILE_SIZE,
                                 TILE_SIZE, TILE_SIZE);
            const QRect tClippedSrcRect = tSrcRect.intersected(srcRect);
            const QRect tileDstRect = tClippedSrcRect.translated(lM, tM);
            const auto srcTile = getTileByIndex(srcCol, srcRow);
            const uint16_t * const srcP = srcTile->data();
            // if no tile data
            if(!srcP) {
                clearRect(tileDstRect, dst);
            } else {
                const QRect relTileRect = tClippedSrcRect.translated(
                                                    -srcCol*TILE_SIZE,
                                                    -srcRow*TILE_SIZE);
                const int minSrcX = relTileRect.left();
                const int maxSrcX = relTileRect.right();
                const int minSrcY = relTileRect.top();
                const int maxSrcY = relTileRect.bottom();
                const int minDstX = tileDstRect.x();
                const int minDstY = tileDstRect.y();
                const int iMax = (maxSrcX - minSrcX + 1)*4; // for every subpixel (4)
                const int jMax = maxSrcY - minSrcY + 1;
                for(int j = 0; j < jMax; j++) {
                    const int srcY = minSrcY + j;
                    const int srcPixelId = srcY*TILE_SIZE + minSrcX;
                    const uint16_t * srcLine = srcP + srcPixelId*4;
                    const int dstY = minDstY + j;
                    const int dstPixelId = dstY*dstWidth + minDstX;
                    uint8_t * dstLine = dstP + dstPixelId*4;
                    for(int i = 0; i < iMax; i++) {
                        const uint32_t val = *srcLine++;
                        *dstLine++ = (val * 255 + (1<<15)/2) / (1<<15);
                    }
                }
            }

        }
    }

    return dst;
}

bool AutoTilesData::drawOnPixmap(SkPixmap &dst, int drawX, int drawY) const {
    drawX -= mZeroTileCol*TILE_SIZE;
    drawY -= mZeroTileRow*TILE_SIZE;
    const qreal qDrawX = drawX;
    const qreal qDrawY = drawY;
    uint8_t * const dstP = static_cast<uint8_t*>(dst.writable_addr());
    if(drawX >= dst.width()) return true;
    if(drawY >= dst.height()) return true;
    if(drawX + width() <= 0) return true;
    if(drawY + height() <= 0) return true;

    const int dstCols = qCeil(static_cast<qreal>(dst.width())/TILE_SIZE);
    const int dstRows = qCeil(static_cast<qreal>(dst.height())/TILE_SIZE);

    const int minDstX = qMax(0, drawX);
    const int minDstY = qMax(0, drawY);
    const int maxDstX = qMin(dst.width() - 1, drawX + width() - 1);
    const int maxDstY = qMin(dst.height() - 1, drawY + height() - 1);

    //const int nX = maxDstX - minDstX;
    //const int nY = maxDstY - minDstY;

    const int minSrcX = qMax(0, qAbs(qMin(0, drawX)));
    //const int minSrcY = qMax(0, qAbs(qMin(0, drawY)));
    //const int maxSrcX = minSrcX + nX;
    //const int maxSrcY = minSrcY + nY;

    const int minDstCol = qMax(0, qCeil(qDrawX/TILE_SIZE));
    const int maxDstCol = qMin(dstCols - 1,
                               minDstCol + mColumnCount - 1);
    const int minDstRow = qMax(0, qCeil(qDrawY/TILE_SIZE));
    const int maxDstRow = qMin(dstRows - 1,
                               minDstRow + mRowCount - 1);

    const int nCols = maxDstCol - minDstCol;
    const int nRows = maxDstRow - minDstRow;

    const int minSrcCol = drawX > 0 ? 0 :
                                      qMax(0, qFloor(qAbs(qDrawX/TILE_SIZE)));
    //const int maxSrcCol = minSrcCol + nCols;

    const int minSrcRow = drawY > 0 ? 0 :
                                      qMax(0, qFloor(qAbs(qDrawY/TILE_SIZE)));
    //const int maxSrcRow = minSrcRow + nRows;
    for(int col = 0; col <= nCols; col++) {
        const int dstCol = minDstCol + col;
        const int srcCol = minSrcCol + col;
        const int dstX0 = dstCol*TILE_SIZE;
        const int minTileDstX = qMax(dstX0, minDstX);
        const int maxTileDstX = qMin((dstCol + 1)*TILE_SIZE - 1, maxDstX);
        const int minSrcTileX = col == 0 ? qMax(0, minSrcX % TILE_SIZE) : 0;
        for(int row = minSrcRow; row <= nRows; row++) {
            const int dstRow = minDstRow + row;
            const int srcRow = minSrcRow + row;
            const auto srcTile = getTileByIndex(srcCol, srcRow);
            const uint16_t * const srcP = srcTile->data();
            const int dstY0 = dstRow*TILE_SIZE;
            const int minTileDstY = qMax(dstY0, minDstY);
            const int maxTileDstY = qMin((dstRow + 1)*TILE_SIZE - 1, maxDstY);
            if(!srcP) {
                for(int y = minTileDstY; y <= maxTileDstY; y++) {
                    uint8_t * dstLine = dstP + (y*dst.width() + minTileDstX)*4;
                    for(int x = minTileDstX; x <= maxTileDstX; x++) {
                        *dstLine++ = 0;
                        *dstLine++ = 0;
                        *dstLine++ = 0;
                        *dstLine++ = 0;
                    }
                }
                continue;
            }
            for(int y = minTileDstY; y <= maxTileDstY; y++) {
                uint8_t * dstLine = dstP + (y*dst.width() + minTileDstX)*4;
                const uint16_t * srcLine = srcP +
                        ((y - dstY0)*TILE_SIZE + minSrcTileX)*4;
                for(int x = minTileDstX; x <= maxTileDstX; x++) {
                    uint32_t r = *srcLine++;
                    uint32_t g = *srcLine++;
                    uint32_t b = *srcLine++;
                    uint32_t a = *srcLine++;

                    // un-premultiply alpha (with rounding)
                    if(a != 0) {
                        r = ((r << 15) + a/2) / a;
                        g = ((g << 15) + a/2) / a;
                        b = ((b << 15) + a/2) / a;
                    } else {
                        r = g = b = 0;
                    }

                    *dstLine++ = (r * 255 + (1<<15)/2) / (1<<15);
                    *dstLine++ = (g * 255 + (1<<15)/2) / (1<<15);
                    *dstLine++ = (b * 255 + (1<<15)/2) / (1<<15);
                    *dstLine++ = (a * 255 + (1<<15)/2) / (1<<15);
                }
            }
        }
    }
    return true;
}

QPoint AutoTilesData::zeroTile() const {
    return QPoint(mZeroTileCol, mZeroTileRow);
}

QPoint AutoTilesData::zeroTilePos() const {
    return zeroTile()*TILE_SIZE;
}

QRect AutoTilesData::pixelBoundingRect() const {
    return tileRectToPixRect(tileBoundingRect());
}

QRect AutoTilesData::tileBoundingRect() const {
    return QRect(-mZeroTileCol, -mZeroTileRow,
                 mColumnCount, mRowCount);
}

QRect AutoTilesData::tileRectToPixRect(const QRect &tileRect) const {
    return QRect(tileRect.x()*TILE_SIZE,
                 tileRect.y()*TILE_SIZE,
                 tileRect.width()*TILE_SIZE,
                 tileRect.height()*TILE_SIZE);
}

QRect AutoTilesData::pixRectToTileRect(const QRect &pixRect) const {
    const int widthRem = pixRect.width() % TILE_SIZE ? 1 : 0;
    const int heightRem = pixRect.height() % TILE_SIZE ? 1 : 0;
    return QRect(pixRect.x()/TILE_SIZE,
                 pixRect.y()/TILE_SIZE,
                 pixRect.width()/TILE_SIZE + widthRem,
                 pixRect.height()/TILE_SIZE + heightRem);
}

void AutoTilesData::setPixelClamp(const QRect &pixRect) {
    mMinCol = qFloor(qreal(pixRect.left())/TILE_SIZE);
    mMaxCol = qCeil(qreal(pixRect.right())/TILE_SIZE);
    mMinRow = qFloor(qreal(pixRect.top())/TILE_SIZE);
    mMaxRow = qCeil(qreal(pixRect.bottom())/TILE_SIZE);
}

void AutoTilesData::swap(AutoTilesData &other) {
    mColumns.swap(other.mColumns);

    std::swap(mMinCol, other.mMinCol);
    std::swap(mMaxCol, other.mMaxCol);
    std::swap(mMinRow, other.mMinRow);
    std::swap(mMaxRow, other.mMaxRow);

    std::swap(mZeroTileCol, other.mZeroTileCol);
    std::swap(mZeroTileRow, other.mZeroTileRow);
    std::swap(mColumnCount, other.mColumnCount);
    std::swap(mRowCount, other.mRowCount);
}

void AutoTilesData::write(eWriteStream& dst) const {
    dst << mZeroTileCol;
    dst << mZeroTileRow;
    dst << mColumnCount;
    dst << mRowCount;
    const int nCols = mColumns.count();
    dst << nCols;
    const int nRows = mColumns.isEmpty() ? 0 : mColumns.first().count();
    dst << nRows;
    for(const auto& col : mColumns) {
        for(const auto& tile : col) {
            tile->write(dst);
        }
    }
}

void AutoTilesData::read(eReadStream &src) {
    clear();
    src >> mZeroTileCol;
    src >> mZeroTileRow;
    src >> mColumnCount;
    src >> mRowCount;
    int nCols;
    src >> nCols;
    int nRows;
    src >> nRows;
    for(int col = 0; col < nCols; col++) {
        mColumns << QList<stdsptr<Tile>>();
        QList<stdsptr<Tile>>& column = mColumns.last();
        for(int row = 0; row < nRows; row++) {
            column << Tile::sRead(src);;
        }
    }
}

void AutoTilesData::discardTransparentTiles() {
    for(QList<stdsptr<Tile>>& col : mColumns) {
        for(auto& tile : col) {
            if(!tile->data()) continue;
            if(!tile->dataTransparent()) continue;
            tile->removeData();
        }
    }
}

bool AutoTilesData::columnEmpty(const int colId) {
    const auto& col = mColumns.at(colId);
    for(const auto& tile : col) {
        if(tile->data()) return false;
    }
    return true;
}

bool AutoTilesData::rowEmpty(const int rowId) {
    for(const auto& col : mColumns) {
        if(col.at(rowId)->data()) return false;
    }
    return true;
}

bool AutoTilesData::cropFirstColumnIfEmpty() {
    if(mColumns.isEmpty()) return false;
    if(!columnEmpty(0)) return false;
    mColumns.removeFirst();
    mColumnCount--;
    mZeroTileCol--;
    return true;
}

bool AutoTilesData::cropLastColumnIfEmpty() {
    if(mColumns.isEmpty()) return false;
    if(!columnEmpty(mColumnCount - 1)) return false;
    mColumns.removeLast();
    mColumnCount--;
    return true;
}

bool AutoTilesData::cropFirstRowIfEmpty() {
    if(mColumns.isEmpty()) return false;
    if(!rowEmpty(0)) return false;
    for(auto& col : mColumns) col.removeFirst();
    mRowCount--;
    mZeroTileRow--;
    return true;
}

bool AutoTilesData::cropLastRowIfEmpty() {
    if(mColumns.isEmpty()) return false;
    if(!rowEmpty(mRowCount - 1)) return false;
    for(auto& col : mColumns) col.removeLast();
    mRowCount--;
    return true;
}

void AutoTilesData::autoCrop() {
    while(cropFirstColumnIfEmpty()) continue;
    while(cropLastColumnIfEmpty()) continue;
    while(cropFirstRowIfEmpty()) continue;
    while(cropLastRowIfEmpty()) continue;
}

stdsptr<Tile> AutoTilesData::requestTile(const int tx, const int ty) {
    stretchToTile(tx, ty);
    return getTile(tx, ty);
}

bool AutoTilesData::stretchToTile(const int tx, const int ty) {
    if(tx > mMaxCol || tx < mMinCol) return false;
    if(ty > mMaxRow || ty < mMinRow) return false;

    if(mColumns.isEmpty()) {
        mZeroTileCol = -tx;
        mZeroTileRow = -ty;
        appendRows(1);
        appendColumns(1);
        return true;
    }

    const int colId = tx + mZeroTileCol;
    const int rowId = ty + mZeroTileRow;

    if(rowId < 0) {
        prependRows(qAbs(rowId));
    } else if(rowId >= mRowCount) {
        appendRows(qAbs(rowId - mRowCount + 1));
    }
    if(colId < 0) {
        prependColumns(qAbs(colId));
    } else if(colId >= mColumnCount) {
        appendColumns(qAbs(colId - mColumnCount + 1));
    }
    return true;
}

void AutoTilesData::replaceTile(const int tx, const int ty,
                                const stdsptr<Tile> &tile) {
    stretchToTile(tx, ty);
    getTile(tx, ty)->copyFrom(*tile);
}

QList<stdsptr<Tile>> AutoTilesData::newColumn() {
    QList<stdsptr<Tile>> col;
    for(int j = 0; j < mRowCount; j++) {
        col.append(mTileCreator(TILE_SPIXEL_SIZE));
    }
    return col;
}

void AutoTilesData::prependRows(const int count) {
    for(QList<stdsptr<Tile>>& col : mColumns) {
        for(int i = 0; i < count; i++) {
            col.prepend(mTileCreator(TILE_SPIXEL_SIZE));
        }
    }
    mRowCount += count;
    mZeroTileRow += count;
}

void AutoTilesData::appendRows(const int count) {
    for(QList<stdsptr<Tile>>& col : mColumns) {
        for(int i = 0; i < count; i++) {
            col.append(mTileCreator(TILE_SPIXEL_SIZE));
        }
    }
    mRowCount += count;
}

void AutoTilesData::prependColumns(const int count) {
    for(int i = 0; i < count; i++) {
        mColumns.prepend(newColumn());
    }
    mColumnCount += count;
    mZeroTileCol += count;
}

void AutoTilesData::appendColumns(const int count) {
    for(int i = 0; i < count; i++) {
        mColumns.append(newColumn());
    }
    mColumnCount += count;
}
