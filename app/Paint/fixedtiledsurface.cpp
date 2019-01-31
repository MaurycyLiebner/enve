#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "colorconversions.h"
#include "fixedtiledsurface.h"
#include "skia/skiahelpers.h"

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
    const auto info = SkiaHelpers::getPremulBGRAInfo(realWidth, realHeight);
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

    uint16_t* tilePointer = nullptr;

    if (tx >= fTilesWidth || ty >= fTilesHeight || tx < 0 || ty < 0) {
        // Give it a tile which we will ignore writes to
        tilePointer = fNullTile;
    } else {
        // Compute the offset for the tile into our linear memory buffer of tiles
        size_t offset = static_cast<size_t>(tx * fTilesHeight + ty) * fTileSize;

        tilePointer = fTileBuffer + offset/sizeof(uint16_t);
    }

    request->buffer = tilePointer;
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

void FixedTiledSurface::updateImage(const int &tx, const int &ty) {
    MyPaintTileRequest request;
    request.tx = tx; request.ty = ty;
    _startRequest(&request);
    int tileSize = fParent.tile_size;
    uint8_t* dst = static_cast<uint8_t*>(fBitmap.getPixels()) +
            (ty*tileSize*fRealWidth + tx*tileSize)*4;
    rgba16_to_rgba8_premultiplied(request.buffer, tileSize,
                   dst, fRealWidth, tileSize);

    _endRequest(&request);
}

void FixedTiledSurface::loadBitmap(const SkBitmap& bitmap) {
    uint8_t * const srcBits = static_cast<uint8_t*>(bitmap.getPixels());
    int tileSize = fParent.tile_size;

    int nCols = fTilesWidth;
    for(int i = 0; i < nCols; i++) {
        uint16_t* dst = fTileBuffer + i*tileSize*fRealHeight*4;
        uint8_t* src = srcBits + i*tileSize*4;
        rgba8_to_rgba16(src, fRealWidth,
                        dst, tileSize,
                        fRealHeight);
    }
}
