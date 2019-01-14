#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "fixedtiledsurface.h"


void freeSimpleTiledSurf(MyPaintSurface *surface) {
    FixedTiledSurface *self = (FixedTiledSurface*)surface;
    self->_free();
}

void tileRequestStart(MyPaintTiledSurface *tiled_surface,
                      MyPaintTileRequest *request) {
    FixedTiledSurface *self = (FixedTiledSurface*)tiled_surface;
    self->_startRequest(request);
}

void tileRequestEnd(MyPaintTiledSurface *tiled_surface,
                    MyPaintTileRequest *request) {
    FixedTiledSurface *self = (FixedTiledSurface*)tiled_surface;
    self->_endRequest(request);
}

void FixedTiledSurface::resetNullTile() {
    if(!fNullTile) {
        int size = fParent.tile_size*fParent.tile_size*4;
        fNullTile = new uint16_t[static_cast<size_t>(size)];
    }
    memset(fNullTile, 0, fTileSize);
}

bool FixedTiledSurface::resize(const int &width, const int &height) {
    assert(width > 0);
    assert(height > 0);

    free(fTileBuffer);

    const int& tileSizePixels = fParent.tile_size;

    const int tilesWidth = qCeil(static_cast<double>(width)/tileSizePixels);
    const int tilesHeight = qCeil(static_cast<double>(height)/tileSizePixels);
    size_t tilesCount = static_cast<size_t>(tilesWidth*tilesHeight);

    const size_t tileValuesCount =
            static_cast<size_t>(tileSizePixels * tileSizePixels * 4);
    const size_t tileByteSize = tileValuesCount * sizeof(uint16_t);
    const size_t bufferSize = tilesCount * tileByteSize;

    const int realWidth = tilesWidth * tileSizePixels;
    const int realHeight = tilesHeight * tileSizePixels;
    assert(realWidth >= width);
    assert(realHeight >= height);
    assert(bufferSize >= static_cast<size_t>(width*height)*4*sizeof(uint16_t));

    uint16_t* buffer = new uint16_t[tilesCount*tileValuesCount];
    if(!buffer) {
        fprintf(stderr, "CRITICAL: unable to allocate enough memory: %zu bytes", bufferSize);
        return false;
    }
    memset(buffer, 0, bufferSize);


    fBufferSize = bufferSize;
    fTileBuffer = buffer;
    fTileSize = tileByteSize;
    fTilesWidth = tilesWidth;
    fTilesHeight = tilesHeight;
    fHeight = height;
    fWidth = width;
    fRealWidth = realWidth;
    fRealHeight = realHeight;
    SkImageInfo info = SkImageInfo::Make(realWidth, realHeight,
                                         kBGRA_8888_SkColorType,
                                         kUnpremul_SkAlphaType,
                                         //kPremul_SkAlphaType,
                                         nullptr);
    fBitmap.allocPixels(info);
    fBitmap.eraseColor(SK_ColorTRANSPARENT);
    resetNullTile();

    return true;
}

bool FixedTiledSurface::initialize(const int &width, const int &height) {
    assert(width > 0);
    assert(height > 0);

    fMyPaintSurface = &fParent.parent;
    mypaint_tiled_surface_init(&fParent,
                               tileRequestStart,
                               tileRequestEnd);
    fParent.parent.destroy = freeSimpleTiledSurf;

    return resize(width, height);
}

FixedTiledSurface::~FixedTiledSurface() {
    _free();
}

void FixedTiledSurface::_startRequest(MyPaintTileRequest * const request) {
    const int tx = request->tx;
    const int ty = request->ty;

    uint16_t* tile_pointer = nullptr;

    if (tx >= fTilesWidth || ty >= fTilesHeight || tx < 0 || ty < 0) {
        // Give it a tile which we will ignore writes to
        tile_pointer = fNullTile;
    } else {
        // Compute the offset for the tile into our linear memory buffer of tiles
        size_t offset = static_cast<size_t>(tx * fTilesHeight + ty) * fTileSize;

        tile_pointer = fTileBuffer + offset/sizeof(uint16_t);
    }

    request->buffer = tile_pointer;
}

void FixedTiledSurface::_endRequest(MyPaintTileRequest * const request) {
    const int tx = request->tx;
    const int ty = request->ty;

    if (tx >= fTilesWidth || ty >= fTilesHeight || tx < 0 || ty < 0) {
        // Wipe any changed done to the null tile
        resetNullTile();
    } else {
        // We hand out direct pointers to our buffer, so for the normal case nothing needs to be done
    }
}

// Naive conversion code from the internal MyPaint format and 8 bit RGB
void fix15_to_rgba8(uint16_t* src,
                    const int& srcWidth,
                    uint8_t* dst,
                    const int& dstWidth,
                    const int& height) {
    for(int i = 0; i < height; i++) {
        uint16_t *srcLine = src + i * srcWidth * 4;
        uint8_t *dstLine = dst + i * dstWidth * 4;
        for(int j = 0; j < srcWidth; j++) {
            uint32_t r, g, b, a;

            r = *srcLine++;
            g = *srcLine++;
            b = *srcLine++;
            a = *srcLine++;

            // un-premultiply alpha (with rounding)
            if(a != 0) {
//                r = ((r << 15)) / a;
//                g = ((g << 15)) / a;
//                b = ((b << 15)) / a;
                r = ((r << 15) + a/2) / a;
                g = ((g << 15) + a/2) / a;
                b = ((b << 15) + a/2) / a;
            } else {
                r = g = b = 0;
            }

            // Variant A) rounding
            const uint32_t add_r = (1<<15)/2;
            const uint32_t add_g = (1<<15)/2;
            const uint32_t add_b = (1<<15)/2;
            const uint32_t add_a = (1<<15)/2;
            *dstLine++ = (r * 255 + add_r) / (1<<15);
            *dstLine++ = (g * 255 + add_g) / (1<<15);
            *dstLine++ = (b * 255 + add_b) / (1<<15);
            *dstLine++ = (a * 255 + add_a) / (1<<15);
//            *dstLine++ = (r * 255) / (1<<15);
//            *dstLine++ = (g * 255) / (1<<15);
//            *dstLine++ = (b * 255) / (1<<15);
//            *dstLine++ = (a * 255) / (1<<15);
        }
    }
}

void FixedTiledSurface::updateImage(const int &tx, const int &ty) {
    MyPaintTileRequest request;
    request.tx = tx; request.ty = ty;
    _startRequest(&request);
    int tileSize = fParent.tile_size;
    uint8_t* dst = static_cast<uint8_t*>(fBitmap.getPixels()) +
            (ty*tileSize*fRealWidth + tx*tileSize)*4;
    fix15_to_rgba8(request.buffer, tileSize,
                   dst, fRealWidth, tileSize);

    _endRequest(&request);
}
