// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

uint16_t *allocateTile(const size_t& size) {
    auto ptr = new uint16_t[size];
    if(!ptr) RuntimeThrow("Could not allocate memory for a tile.");
    return ptr;
}

uint16_t *newZeroedTile(const size_t& size) {
    auto ptr = new uint16_t[size];
    if(!ptr) RuntimeThrow("Could not allocate memory for a tile.");
    memset(ptr, 0, size*sizeof(uint16_t));
    return ptr;
}

AutoTilesData::AutoTilesData() {}

AutoTilesData::AutoTilesData(const AutoTilesData &other) {
    mZeroTileCol = other.mZeroTileCol;
    mZeroTileRow = other.mZeroTileRow;
    mColumnCount = other.mColumnCount;
    mRowCount = other.mRowCount;

    for(auto& column : other.mColumns) {
        mColumns.append(QList<uint16_t*>());
        QList<uint16_t*> &col = mColumns.last();
        for(auto& srcTile : column) {
            const auto dstTile = allocateTile(TILE_SPIXEL_SIZE);
            uint16_t * dstP = dstTile;
            const uint16_t * srcP = srcTile;
            for(int sp = 0; sp < TILE_SPIXEL_SIZE; sp++) {
                *(dstP++) = *(srcP++);
            }

            col << dstTile;
        }
    }
}

AutoTilesData::AutoTilesData(AutoTilesData &&other) {
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
        QList<uint16_t*> colRows;
        for(int row = 0; row < nRows; row++) {
            const bool lastRow = row == (nRows - 1);
            const bool iniZeroed = lastCol || lastRow;
            uint16_t * tileP;
            try {
                tileP = iniZeroed ? newZeroedTile(TILE_SPIXEL_SIZE) :
                                    allocateTile(TILE_SPIXEL_SIZE);
            } catch(...) {
                clear();
                RuntimeThrow("Failed to load bitmap to AutoTilesData.");
            }

            const int y0 = row*TILE_SIZE;
            const int maxY = qMin(y0 + TILE_SIZE, src.height());
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
            colRows << tileP;
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
    for(const auto& col : mColumns) {
        for(const auto& row : col) {
            free(row);
        }
    }
    mColumns.clear();
    mZeroTileCol = 0;
    mZeroTileRow = 0;
    mColumnCount = 0;
    mRowCount = 0;
}

uint16_t *AutoTilesData::getTile(const int tx, const int ty) const {
    return getTileByIndex(tx + mZeroTileCol, ty + mZeroTileRow);
}

uint16_t *AutoTilesData::getTileByIndex(const int colId,
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

void AutoTilesData::tileToBitmap(const int tx, const int ty, SkBitmap &bitmap) {
    Q_ASSERT(bitmap.width() == TILE_SIZE);
    Q_ASSERT(bitmap.height() == TILE_SIZE);
    uint8_t * const dstP = static_cast<uint8_t*>(bitmap.getPixels());
    Q_ASSERT(dstP);
    const uint16_t * const srcP = getTile(tx, ty);

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
}

SkBitmap AutoTilesData::tileToBitmap(const int tx, const int ty) {
    const auto info = SkiaHelpers::getPremulRGBAInfo(TILE_SIZE, TILE_SIZE);
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    tileToBitmap(tx, ty, bitmap);
    return bitmap;
}

SkBitmap AutoTilesData::toBitmap(const QMargins& margin) const {
    const int lM = margin.left();
    const int tM = margin.top();
    const int rM = margin.right();
    const int bM = margin.bottom();

    const int lM0 = qMax(0, lM);
    const int tM0 = qMax(0, tM);
    const int rM0 = qMax(0, rM);
    const int bM0 = qMax(0, bM);

    const auto info = SkiaHelpers::getPremulRGBAInfo(width() + lM + rM,
                                                     height() + tM + bM);
    SkBitmap dst;
    dst.allocPixels(info);

    uint8_t * const dstP = static_cast<uint8_t*>(dst.getPixels());

//    if(tM0 > 0 || rM0 > 0) {
//        for(int y = 0; y < dst.height(); y++) {
//            auto lLine = dstP + y*dst.width()*4;
//            for(int x = 0; x < tM0; x++) {
//                *lLine++ = 0;
//                *lLine++ = 0;
//                *lLine++ = 0;
//                *lLine++ = 0;
//            }

//            auto rLine = dstP + ((y + 1)*dst.width() - rM0)*4;
//            for(int x = 0; x < rM0; x++) {
//                *rLine++ = 0;
//                *rLine++ = 0;
//                *rLine++ = 0;
//                *rLine++ = 0;
//            }
//        }
//    }

//    for(int y = 0; y < tM0; y++) {
//        auto tLine = dstP + (y*dst.width() + lM0)*4;
//        for(int x = lM0; x < dst.width() - rM0; x++) {
//            *tLine++ = 0;
//            *tLine++ = 0;
//            *tLine++ = 0;
//            *tLine++ = 0;
//        }
//    }

//    for(int y = dst.height() - bM0; y < dst.height(); y++) {
//        auto bLine = dstP + (y*dst.width() + lM0)*4;
//        for(int x = lM0; x < dst.width() - rM0; x++) {
//            *bLine++ = 0;
//            *bLine++ = 0;
//            *bLine++ = 0;
//            *bLine++ = 0;
//        }
//    }

    const int minY = -mZeroTileRow*TILE_SIZE + qMax(0, -tM);
    const int minX = -mZeroTileCol*TILE_SIZE + qMax(0, -lM);
    const int minCol = mZeroTileCol + minX/TILE_SIZE;
    const int maxCol = mColumnCount - qMax(0, -rM/TILE_SIZE);
    const int minRow = mZeroTileRow + minY/TILE_SIZE;
    const int maxRow = mRowCount - qMax(0, -bM/TILE_SIZE);
    for(int col = minCol; col < maxCol; col++) {
        const int x0 = col*TILE_SIZE + lM;
        const int maxX = qMin(x0 + TILE_SIZE, dst.width() - rM0);
        for(int row = minRow; row < maxRow; row++) {
            const uint16_t * const srcP = getTileByIndex(col, row);
            const int y0 = row*TILE_SIZE + tM;
            const int maxY = qMin(y0 + TILE_SIZE, dst.height() - bM0);
            for(int y = qMax(minY, y0); y < maxY; y++) {
                uint8_t * dstLine = dstP + (y*dst.width() + x0)*4 + minX - qMin(minX, x0);
                const uint16_t * srcLine = srcP + (y - y0)*TILE_SIZE*4 + minX - qMin(minX, x0);
                for(int x = qMax(minX, x0); x < maxX; x++) {
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
        }
    }
    return dst;
}

bool AutoTilesData::drawOnPixmap(SkPixmap &dst,
                                 int drawX, int drawY) const {
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
            const uint16_t * const srcP = getTileByIndex(srcCol, srcRow);
            const int dstY0 = dstRow*TILE_SIZE;
            const int minTileDstY = qMax(dstY0, minDstY);
            const int maxTileDstY = qMin((dstRow + 1)*TILE_SIZE - 1, maxDstY);
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
        for(const auto& row : col) {
            dst.write(row, TILE_SPIXEL_SIZE*sizeof(uint16_t));
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
        mColumns << QList<uint16_t*>();
        QList<uint16_t*>& column = mColumns.last();
        for(int row = 0; row < nRows; row++) {
            const auto tile = allocateTile(TILE_SPIXEL_SIZE);
            src.read(tile, TILE_SPIXEL_SIZE*sizeof(uint16_t));
            column << tile;
        }
    }
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

QList<uint16_t *> AutoTilesData::newColumn() {
    QList<uint16_t*> col;
    for(int j = 0; j < mRowCount; j++) {
        col << newZeroedTile(TILE_SPIXEL_SIZE);
    }
    return col;
}

void AutoTilesData::prependRows(const int count) {
    for(QList<uint16_t*>& col : mColumns) {
        for(int i = 0; i < count; i++) {
            col.prepend(newZeroedTile(TILE_SPIXEL_SIZE));
        }
    }
    mRowCount += count;
    mZeroTileRow += count;
}

void AutoTilesData::appendRows(const int count) {
    for(QList<uint16_t*>& col : mColumns) {
        for(int i = 0; i < count; i++) {
            col.append(newZeroedTile(TILE_SPIXEL_SIZE));
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
