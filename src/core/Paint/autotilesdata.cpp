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

#include <QImage>

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

template <void (*Swapper)(uint8_t&, uint8_t&, uint8_t&, uint8_t&)>
void unpremul_8_to_15(uint8_t const *& srcLine, uint16_t*& dstLine) {
    uint8_t r = *srcLine++;
    uint8_t g = *srcLine++;
    uint8_t b = *srcLine++;
    uint8_t a = *srcLine++;
    Swapper(r, g, b, a);

    uint32_t r32 = r;
    uint32_t g32 = g;
    uint32_t b32 = b;
    uint32_t a32 = a;

    // convert to fixed point (with rounding)
    r32 = (r * (1<<15) + 255/2) / 255;
    g32 = (g * (1<<15) + 255/2) / 255;
    b32 = (b * (1<<15) + 255/2) / 255;
    a32 = (a * (1<<15) + 255/2) / 255;

    // premultiply alpha (with rounding), save back
    *dstLine++ = (r32 * a32 + (1<<15)/2) / (1<<15);
    *dstLine++ = (g32 * a32 + (1<<15)/2) / (1<<15);
    *dstLine++ = (b32 * a32 + (1<<15)/2) / (1<<15);
    *dstLine++ = a32;
}

template <void (*Swapper)(uint8_t&, uint8_t&, uint8_t&, uint8_t&)>
void opaque_8_to_15(uint8_t const *& srcLine, uint16_t*& dstLine) {
    uint8_t r = *srcLine++;
    uint8_t g = *srcLine++;
    uint8_t b = *srcLine++;
    srcLine++;
    uint8_t a = 255;
    Swapper(r, g, b, a);

    // convert to fixed point (with rounding)
    *dstLine++ = (r * (1<<15) + 255/2) / 255;
    *dstLine++ = (g * (1<<15) + 255/2) / 255;
    *dstLine++ = (b * (1<<15) + 255/2) / 255;
    *dstLine++ = (a * (1<<15) + 255/2) / 255;
}

template <void (*Swapper)(uint8_t&, uint8_t&, uint8_t&, uint8_t&)>
void premul_8_to_15(uint8_t const *& srcLine, uint16_t*& dstLine) {
    uint8_t r = *srcLine++;
    uint8_t g = *srcLine++;
    uint8_t b = *srcLine++;
    uint8_t a = *srcLine++;
    Swapper(r, g, b, a);

    // convert to fixed point (with rounding)
    *dstLine++ = (r * (1<<15) + 255/2) / 255;
    *dstLine++ = (g * (1<<15) + 255/2) / 255;
    *dstLine++ = (b * (1<<15) + 255/2) / 255;
    *dstLine++ = (a * (1<<15) + 255/2) / 255;
}

template <void (*Swapper)(uint16_t&, uint16_t&, uint16_t&, uint16_t&)>
void unpremul_16_to_15(uint16_t const *& srcLine, uint16_t*& dstLine) {
    uint16_t r = *srcLine++;
    uint16_t g = *srcLine++;
    uint16_t b = *srcLine++;
    uint16_t a = *srcLine++;
    Swapper(r, g, b, a);

    uint32_t r32 = r;
    uint32_t g32 = g;
    uint32_t b32 = b;
    uint32_t a32 = a;

    // convert to fixed point (with rounding)
    r32 = (r32 * (1<<15) + USHRT_MAX/2) / USHRT_MAX;
    g32 = (g32 * (1<<15) + USHRT_MAX/2) / USHRT_MAX;
    b32 = (b32 * (1<<15) + USHRT_MAX/2) / USHRT_MAX;
    a32 = (a32 * (1<<15) + USHRT_MAX/2) / USHRT_MAX;

    // premultiply alpha (with rounding), save back
    *dstLine++ = (r32 * a32 + (1<<15)/2) / (1<<15);
    *dstLine++ = (g32 * a32 + (1<<15)/2) / (1<<15);
    *dstLine++ = (b32 * a32 + (1<<15)/2) / (1<<15);
    *dstLine++ = a32;
}


template <void (*Swapper)(uint16_t&, uint16_t&, uint16_t&, uint16_t&)>
void opaque_16_to_15(uint16_t const *& srcLine, uint16_t*& dstLine) {
    uint16_t r = *srcLine++;
    uint16_t g = *srcLine++;
    uint16_t b = *srcLine++;
    srcLine++;
    uint16_t a = USHRT_MAX;
    Swapper(r, g, b, a);

    // convert to fixed point (with rounding)
    *dstLine++ = (r * (1<<15) + 255/2) / 255;
    *dstLine++ = (g * (1<<15) + 255/2) / 255;
    *dstLine++ = (b * (1<<15) + 255/2) / 255;
    *dstLine++ = (a * (1<<15) + 255/2) / 255;
}

template <void (*Swapper)(uint16_t&, uint16_t&, uint16_t&, uint16_t&)>
void premul_16_to_15(uint16_t const *& srcLine, uint16_t*& dstLine) {
    uint16_t r = *srcLine++;
    uint16_t g = *srcLine++;
    uint16_t b = *srcLine++;
    uint16_t a = *srcLine++;
    Swapper(r, g, b, a);

    // convert to fixed point (with rounding)
    *dstLine++ = (r * (1<<15) + USHRT_MAX/2) / USHRT_MAX;
    *dstLine++ = (g * (1<<15) + USHRT_MAX/2) / USHRT_MAX;
    *dstLine++ = (b * (1<<15) + USHRT_MAX/2) / USHRT_MAX;
    *dstLine++ = (a * (1<<15) + USHRT_MAX/2) / USHRT_MAX;
}

template <typename Addr, void (*To15Bit)(Addr const *& srcLine, uint16_t*& dstLine)>
void AutoTilesData::loadPixmap(const Addr * const src, const int width, const int height) {
    clear();
    const int nCols = qCeil(static_cast<qreal>(width)/TILE_SIZE);
    const int nRows = qCeil(static_cast<qreal>(height)/TILE_SIZE);

    for(int col = 0; col < nCols; col++) {
        const bool lastCol = col == (nCols - 1);
        const int x0 = col*TILE_SIZE;
        const int maxX = qMin(x0 + TILE_SIZE, width);
        QList<stdsptr<Tile>> colRows;
        for(int row = 0; row < nRows; row++) {
            const auto tile = mTileCreator(TILE_SPIXEL_SIZE);

            const bool lastRow = row == (nRows - 1);
            const bool iniZeroed = lastCol || lastRow;
            if(iniZeroed) tile->zeroData();
            const auto tileP = tile->requestData();

            const int y0 = row*TILE_SIZE;
            const int maxY = qMin(y0 + TILE_SIZE, height);
            for(int y = y0; y < maxY; y++) {
                const Addr * srcLine = src + (y*width + x0)*4;
                uint16_t* dstLine = tileP + (y - y0)*TILE_SIZE*4;
                for(int x = x0; x < maxX; x++) {
                    To15Bit(srcLine, dstLine);
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

template <typename T>
inline void BGRA_to_RGBA(T& b, T& g, T& r, T& a) {
    Q_UNUSED(g)
    Q_UNUSED(a)
    std::swap(b, r);
}

template <typename T>
inline void RGBA_to_RGBA(T& r, T& g, T& b, T& a) {
    Q_UNUSED(r)
    Q_UNUSED(g)
    Q_UNUSED(b)
    Q_UNUSED(a)
}

template <typename T>
inline void ARGB_to_RGBA(T& a, T& r, T& g, T& b) {
    const T rr = r;
    const T gg = g;
    const T bb = b;
    const T aa = a;
    a = rr;
    r = gg;
    g = bb;
    b = aa;
}

template <void (*Swapper)(uint8_t&, uint8_t&, uint8_t&, uint8_t&)>
void AutoTilesData::loadPixmap_XXXA_8888(const uint8_t* const addr8,
                                         const int width,
                                         const int height,
                                         const SkAlphaType alphaType) {
    if(alphaType == kUnpremul_SkAlphaType) {
        loadPixmap<uint8_t, unpremul_8_to_15<Swapper>>(addr8, width, height);
    } else if(alphaType == kPremul_SkAlphaType) {
        loadPixmap<uint8_t, premul_8_to_15<Swapper>>(addr8, width, height);
    } else if(alphaType == kOpaque_SkAlphaType) {
        loadPixmap<uint8_t, opaque_8_to_15<Swapper>>(addr8, width, height);
    } else RuntimeThrow("Unsupported alpha type");
}

template <void (*Swapper)(uint16_t&, uint16_t&, uint16_t&, uint16_t&)>
void AutoTilesData::loadPixmap_XXXA_16161616(const uint16_t* const addr16,
                                             const int width,
                                             const int height,
                                             const SkAlphaType alphaType) {
    if(alphaType == kUnpremul_SkAlphaType) {
        loadPixmap<uint16_t, unpremul_16_to_15<Swapper>>(addr16, width, height);
    } else if(alphaType == kPremul_SkAlphaType) {
        loadPixmap<uint16_t, premul_16_to_15<Swapper>>(addr16, width, height);
    } else if(alphaType == kOpaque_SkAlphaType) {
        loadPixmap<uint16_t, opaque_16_to_15<Swapper>>(addr16, width, height);
    } else RuntimeThrow("Unsupported alpha type");
}

void AutoTilesData::loadPixmap(const QImage &src) {
    const int width = src.width();
    const int height = src.height();
    const auto format = src.format();
    const auto pixelFormat = src.pixelFormat();
    const auto formatPremultiplied = pixelFormat.premultiplied();
    SkAlphaType alphaType;
    switch(formatPremultiplied) {
    case QPixelFormat::NotPremultiplied:
        alphaType = SkAlphaType::kUnpremul_SkAlphaType;
        break;
    case QPixelFormat::Premultiplied:
        alphaType = SkAlphaType::kPremul_SkAlphaType;
        break;
    }

    if(format == QImage::Format_RGBA8888 ||
       format == QImage::Format_RGBA8888_Premultiplied) {
        loadPixmap_XXXA_8888<RGBA_to_RGBA>(src.bits(), width, height, alphaType);
    } else if(format == QImage::Format_ARGB32 ||
              format == QImage::Format_ARGB32_Premultiplied) {
        loadPixmap_XXXA_8888<BGRA_to_RGBA>(src.bits(), width, height, alphaType);
    } else if(format == QImage::Format_RGBA64 ||
              format == QImage::Format_RGBA64_Premultiplied) {
        const auto addr16 = reinterpret_cast<const uint16_t*>(src.bits());
        loadPixmap_XXXA_16161616<RGBA_to_RGBA>(addr16, width, height, alphaType);
    } else {
        const QImage converted = src.convertToFormat(
                    QImage::Format_RGBA64_Premultiplied);
        loadPixmap(converted);
    }
}

void AutoTilesData::loadPixmap(const SkPixmap &src) {
    const int width = src.width();
    const int height = src.height();
    const auto& info = src.info();
    const SkAlphaType alphaType = info.alphaType();
    const SkColorType colorType = info.colorType();
    if(colorType == SkColorType::kRGBA_8888_SkColorType) {
        const auto addr8 = static_cast<const uint8_t*>(src.addr());
        loadPixmap_XXXA_8888<RGBA_to_RGBA>(addr8, width, height, alphaType);
    } else if(colorType == SkColorType::kBGRA_8888_SkColorType) {
        const auto addr8 = static_cast<const uint8_t*>(src.addr());
        loadPixmap_XXXA_8888<BGRA_to_RGBA>(addr8, width, height, alphaType);
    } else {
        SkBitmap dst;
        dst.allocPixels(SkiaHelpers::getPremulRGBAInfo(width, height));
        if(dst.writePixels(src)) {
            loadPixmap(dst.pixmap());
        } else {
            dst.eraseColor(SK_ColorTRANSPARENT);
            SkCanvas canvas(dst);
            SkBitmap srcBitmap;
            srcBitmap.installPixels(src);
            canvas.drawBitmap(srcBitmap, 0, 0);
            canvas.flush();
            loadPixmap(dst.pixmap());
        }
    }
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

template<typename Addr>
void clearRect(const QRect& rect, const int dstWidth, Addr * const dst) {
    for(int y = rect.top(); y <= rect.bottom(); y++) {
        const int firstRowPixelId = y*dstWidth + rect.left();
        Addr * dstLine = dst + firstRowPixelId*4;
        for(int x = rect.left(); x <= rect.right(); x++) {
            *dstLine++ = 0;
            *dstLine++ = 0;
            *dstLine++ = 0;
            *dstLine++ = 0;
        }
    }
}

template<typename Addr>
void clearMarginPixels(const QMargins& margin,
                       const int dstWidth, const int dstHeight,
                       Addr * const dst) {
    const int lM = qMax(0, margin.left());
    const int tM = qMax(0, margin.top());
    const int rM = qMax(0, margin.right());
    const int bM = qMax(0, margin.bottom());

    clearRect(QRect(0, 0, lM - 1, dstHeight - 1), dstWidth, dst);
    clearRect(QRect(dstWidth - rM, 0, rM - 1, dstHeight - 1), dstWidth, dst);
    clearRect(QRect(lM, 0, dstWidth - lM - rM - 1, tM - 1), dstWidth, dst);
    clearRect(QRect(lM, dstHeight - bM, dstWidth - lM - rM - 1, bM - 1), dstWidth, dst);
}

void premul_15_to_permul_8(uint8_t* dstLine, const uint16_t* srcLine) {
    const uint32_t src = *srcLine;
    *dstLine = (src * 255 + (1<<15)/2) / (1<<15);
}

void premul_15_to_permul_16(uint16_t* dstLine, const uint16_t* srcLine) {
    const uint32_t src = *srcLine;
    *dstLine = (src * USHRT_MAX + (1<<15)/2) / (1<<15);
}

template<typename Addr, void (*From15Bit)(Addr *dstLine, const uint16_t* srcLine)>
void AutoTilesData::toBitmap(Addr * const dst, const QMargins &margin,
                             const int dstWidth, const int dstHeight) const {
    const int lM = margin.left();
    const int tM = margin.top();
    const int rM = margin.right();
    const int bM = margin.bottom();

    clearMarginPixels(margin, dstWidth, dstHeight, dst);

    const QPoint dstTL(qMax(0, lM),
                       qMax(0, tM));
    const QPoint dstBR(dstWidth  - 1 - qMax(0, rM),
                       dstHeight - 1 - qMax(0, bM));

    const QRect dstRect(dstTL, dstBR);
    const QPoint srcTL(qMax(0, -lM),
                       qMax(0, -tM));
    const QPoint srcBR(width()  - 1 - qMax(0, -rM),
                       height() - 1 - qMax(0, -bM));
    const QRect srcRect(srcTL, srcBR);
    if(!srcRect.isValid()) return;
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
                clearRect(tileDstRect, dstWidth, dst);
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
                    Addr * dstLine = dst + dstPixelId*4;
                    for(int i = 0; i < iMax; i++) {
                        From15Bit(dstLine++, srcLine++);
                    }
                }
            }

        }
    }
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
    uint8_t * const dstP = static_cast<uint8_t*>(dst.getPixels());
    toBitmap<uint8_t, premul_15_to_permul_8>(dstP, margin, dstWidth, dstHeight);
    return dst;
}

QImage AutoTilesData::toImage(const bool use16Bit,
                              const QMargins &margin) const {
    const int lM = margin.left();
    const int tM = margin.top();
    const int rM = margin.right();
    const int bM = margin.bottom();

    const int dstWidth  = width()  + lM + rM;
    const int dstHeight = height() + tM + bM;
    const auto info = SkiaHelpers::getPremulRGBAInfo(dstWidth, dstHeight);
    QImage dst;
    if(use16Bit) {
        dst = QImage(dstWidth, dstHeight, QImage::Format_RGBA64_Premultiplied);
        uint16_t * const dstP = reinterpret_cast<uint16_t*>(dst.bits());
        toBitmap<uint16_t, premul_15_to_permul_16>(dstP, margin, dstWidth, dstHeight);
    } else {
        dst = QImage(dstWidth, dstHeight, QImage::Format_RGBA8888_Premultiplied);
        uint8_t * const dstP = static_cast<uint8_t*>(dst.bits());
        toBitmap<uint8_t, premul_15_to_permul_8>(dstP, margin, dstWidth, dstHeight);
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
            column << Tile::sRead(src, mTileCreator);
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

void AutoTilesData::removeFirstColumn() {
    mColumns.removeFirst();
    mColumnCount--;
    mZeroTileCol--;
}

void AutoTilesData::removeLastColumn() {
    mColumns.removeLast();
    mColumnCount--;
}

void AutoTilesData::removeFirstRow() {
    for(auto& col : mColumns) col.removeFirst();
    mRowCount--;
    mZeroTileRow--;
}

void AutoTilesData::removeLastRow() {
    for(auto& col : mColumns) col.removeLast();
    mRowCount--;
}

void AutoTilesData::zeroFirstColumn() {
    const auto& col = mColumns.first();
    for(const auto& tile : col) tile->zeroData();
}

void AutoTilesData::zeroLastColumn() {
    const auto& col = mColumns.last();
    for(const auto& tile : col) tile->zeroData();
}

void AutoTilesData::zeroFirstRow() {
    for(auto& col : mColumns) col.first()->zeroData();
}

void AutoTilesData::zeroLastRow() {
    for(auto& col : mColumns) col.last()->zeroData();
}

bool AutoTilesData::cropFirstColumnIfEmpty() {
    if(mColumns.isEmpty()) return false;
    if(!columnEmpty(0)) return false;
    removeFirstColumn();
    return true;
}

bool AutoTilesData::cropLastColumnIfEmpty() {
    if(mColumns.isEmpty()) return false;
    if(!columnEmpty(mColumnCount - 1)) return false;
    removeLastColumn();
    return true;
}

bool AutoTilesData::cropFirstRowIfEmpty() {
    if(mColumns.isEmpty()) return false;
    if(!rowEmpty(0)) return false;
    removeFirstRow();
    return true;
}

bool AutoTilesData::cropLastRowIfEmpty() {
    if(mColumns.isEmpty()) return false;
    if(!rowEmpty(mRowCount - 1)) return false;
    removeLastRow();
    return true;
}

void AutoTilesData::autoCrop() {
    discardTransparentTiles();
    while(cropFirstColumnIfEmpty()) continue;
    while(cropLastColumnIfEmpty()) continue;
    while(cropFirstRowIfEmpty()) continue;
    while(cropLastRowIfEmpty()) continue;
}

void AutoTilesData::crop(const QRect &cropRect) {
    const QRect iniRect = pixelBoundingRect();
    const QRect normalizedCrop = cropRect.normalized();
    const QRect clampedCrop = normalizedCrop.intersected(iniRect);
    if(!clampedCrop.isValid()) return;

    {
        const int dl = clampedCrop.left() - iniRect.left();
        const int dtl = dl/TILE_SIZE;
        const int dpl = dl - dtl*TILE_SIZE;

        moveX(-dpl, false);
        for(int i = 0; i < dtl; i++) {
            zeroFirstColumn();
            removeFirstColumn();
        }
        moveX(dpl, false);
    }

    {
        const int dr = clampedCrop.right() - iniRect.right();
        const int dtr = dr/TILE_SIZE;
        const int dpr = dr - dtr*TILE_SIZE;

        moveX(-dpr, false);
        for(int i = 0; i < -dtr; i++) {
            zeroLastColumn();
            removeLastColumn();
        }
        moveX(dpr, false);
    }

    {
        const int dt = clampedCrop.top() - iniRect.top();
        const int dtt = dt/TILE_SIZE;
        const int dpt = dt - dtt*TILE_SIZE;

        moveY(-dpt, false);
        for(int i = 0; i < dtt; i++) {
            zeroFirstRow();
            removeFirstRow();
        }
        moveY(dpt, false);
    }

    {
        const int db = clampedCrop.bottom() - iniRect.bottom();
        const int dtb = db/TILE_SIZE;
        const int dpb = db - dtb*TILE_SIZE;

        moveY(-dpb, false);
        for(int i = 0; i < -dtb; i++) {
            zeroLastRow();
            removeLastRow();
        }
        moveY(dpb, false);
    }

    autoCrop();
}

void AutoTilesData::moveX(const int dx, const bool extend) {
    const int dtx = dx/TILE_SIZE;
    const int dpx = dx - dtx*TILE_SIZE;

    mZeroTileCol -= dtx;

    if(dpx == 0) return;

    if(extend) {
        prependColumns(1);
        appendColumns(1);
    }

    const auto emptyTile = mTileCreator(TILE_SPIXEL_SIZE);
    emptyTile->requestZeroedData();

    if(dpx > 0) {
        for(int i = mColumnCount - 1; i >= 0; i--) {
            const bool isFirst = i == 0;
            const auto& col = mColumns.at(i);
            const auto& prevCol = isFirst ? col : mColumns.at(i - 1);
            for(int j = 0; j < mRowCount; j++) {
                const auto& dstTile = col.at(j);
                // move pixels inside tile
                if(uint16_t* const dstData = dstTile->data()) {
                    const int dstXDP = TILE_SIZE*4;
                    const int srcXDP = (TILE_SIZE - dpx)*4;
                    for(int y = 0; y < TILE_SIZE; y++) {
                        const int rowDP = y*TILE_SIZE*4;
                        uint16_t* dst = dstData + rowDP + dstXDP;
                        uint16_t* src = dstData + rowDP + srcXDP;
                        for(int dstX = TILE_SIZE - 1; dstX >= dpx; dstX--) {
                            for(int sp = 0; sp < 4; sp++) *(--dst) = *(--src);
                        }
                    }
                }
                uint16_t* const dstData = dstTile->requestZeroedData();

                const auto& srcTile = isFirst ? emptyTile : prevCol.at(j);
                uint16_t* const srcData = srcTile->requestZeroedData();

                // move pixels from the previous tile
                const int srcXDP = (TILE_SIZE - dpx)*4;
                for(int y = 0; y < TILE_SIZE; y++) {
                    const int rowDP = y*TILE_SIZE*4;
                    uint16_t* dst = dstData + rowDP;
                    uint16_t* src = srcData + rowDP + srcXDP;
                    for(int dstX = 0; dstX < dpx; dstX++) {
                        for(int sp = 0; sp < 4; sp++) *(dst++) = *(src++);
                    }
                }
            }
        }
    } else if(dpx < 0) {
        for(int i = 0; i < mColumnCount; i++) {
            const bool isLast = i == (mColumnCount - 1);
            const auto& col = mColumns.at(i);
            const auto& nextCol = isLast ? col : mColumns.at(i + 1);
            for(int j = 0; j < mRowCount; j++) {
                const auto& dstTile = col.at(j);
                // move pixels inside tile
                const int maxX = TILE_SIZE + dpx;
                if(uint16_t* const dstData = dstTile->data()) {
                    const int srcXDP = -dpx*4;
                    for(int y = 0; y < TILE_SIZE; y++) {
                        const int rowDP = y*TILE_SIZE*4;
                        uint16_t* dst = dstData + rowDP;
                        uint16_t* src = dst + srcXDP;

                        for(int dstX = 0; dstX < maxX; dstX++) {
                            for(int sp = 0; sp < 4; sp++) *(dst++) = *(src++);
                        }
                    }
                }

                uint16_t* const dstData = dstTile->requestZeroedData();

                const auto& srcTile = isLast ? emptyTile : nextCol.at(j);
                uint16_t* const srcData = srcTile->requestZeroedData();

                // move pixels from the next tile
                const int dstX0 = TILE_SIZE + dpx;
                const int dstXDP = dstX0*4;
                for(int y = 0; y < TILE_SIZE; y++) {
                    const int rowDP = y*TILE_SIZE*4;
                    uint16_t* dst = dstData + rowDP + dstXDP;
                    uint16_t* src = srcData + rowDP;
                    for(int dstX = dstX0; dstX < TILE_SIZE; dstX++) {
                        for(int sp = 0; sp < 4; sp++) *(dst++) = *(src++);
                    }
                }
            }
        }
    }
}

void AutoTilesData::moveY(const int dy, const bool extend) {
    const int dty = dy/TILE_SIZE;
    const int dpy = dy - dty*TILE_SIZE;

    mZeroTileRow -= dty;

    if(dpy == 0) return;

    if(extend) {
        prependRows(1);
        appendRows(1);
    }

    const auto emptyTile = mTileCreator(TILE_SPIXEL_SIZE);
    emptyTile->requestZeroedData();

    if(dpy > 0) {
        for(const auto& col : mColumns) {
            for(int j = mRowCount - 1; j >= 0; j--) {
                const auto& dstTile = col.at(j);
                // move pixels inside tile
                if(uint16_t* const dstData = dstTile->data()) {
                    for(int dstY = TILE_SIZE - 1; dstY >= dpy; dstY--) {
                        uint16_t* dst = dstData + dstY*TILE_SIZE*4;
                        uint16_t* src = dstData + (dstY - dpy)*TILE_SIZE*4;
                        for(int x = 0; x < TILE_SIZE; x++) {
                            for(int sp = 0; sp < 4; sp++) *(dst++) = *(src++);
                        }
                    }
                }

                uint16_t* const dstData = dstTile->requestZeroedData();

                const bool isFirst = j == 0;
                const auto& srcTile = isFirst ? emptyTile : col.at(j - 1);
                uint16_t* const srcData = srcTile->requestZeroedData();

                // move pixels from the previous tile
                uint16_t* dst = dstData;
                uint16_t* src = srcData + (TILE_SIZE - dpy)*TILE_SIZE*4;
                for(int dstY = 0; dstY < dpy; dstY++) {
                    for(int x = 0; x < TILE_SIZE; x++) {
                        for(int sp = 0; sp < 4; sp++) *(dst++) = *(src++);
                    }
                }
            }
        }
    } else if(dpy < 0) {
        for(const auto& col : mColumns) {
            for(int j = 0; j < mRowCount; j++) {
                const auto& dstTile = col.at(j);
                // move pixels inside tile
                if(uint16_t* const dstData = dstTile->data()) {
                    const int maxY = TILE_SIZE + dpy;
                    uint16_t* dst = dstData;
                    uint16_t* src = dstData - dpy*TILE_SIZE*4;
                    for(int dstY = 0; dstY < maxY; dstY++) {
                        for(int x = 0; x < TILE_SIZE; x++) {
                            for(int sp = 0; sp < 4; sp++) *(dst++) = *(src++);
                        }
                    }
                }

                // the last row has no next row to get data from
                uint16_t* const dstData = dstTile->requestZeroedData();

                const bool isLast = j == (mRowCount - 1);
                const auto& srcTile = isLast ? emptyTile : col.at(j + 1);
                uint16_t* const srcData = srcTile->requestZeroedData();

                // move pixels from the next tile
                const int dstY0 = TILE_SIZE + dpy;
                uint16_t* dst = dstData + dstY0*TILE_SIZE*4;
                uint16_t* src = srcData;
                for(int dstY = dstY0; dstY < TILE_SIZE; dstY++) {
                    for(int x = 0; x < TILE_SIZE; x++) {
                        for(int sp = 0; sp < 4; sp++) *(dst++) = *(src++);
                    }
                }
            }
        }
    }
}

void AutoTilesData::move(const int dx, const int dy) {
    moveX(dx, true);
    moveY(dy, true);
    if(dx % TILE_SIZE != 0 || dy % TILE_SIZE != 0) autoCrop();
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
