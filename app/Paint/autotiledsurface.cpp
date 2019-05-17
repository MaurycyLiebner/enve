#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "colorconversions.h"
#include "autotiledsurface.h"

void autoTiledSurfaceFree(MyPaintSurface *surface) {
    const auto self = reinterpret_cast<AutoTiledSurface*>(surface);
    self->_free();
}

void autoTiledSurfaceRequestStart(MyPaintTiledSurface *tiled_surface,
                                  MyPaintTileRequest *request) {
    const auto self = reinterpret_cast<AutoTiledSurface*>(tiled_surface);
    self->_startRequest(request);
}

void autoTiledSurfaceRequestEnd(MyPaintTiledSurface *tiled_surface,
                                MyPaintTileRequest *request) {
    const auto self = reinterpret_cast<AutoTiledSurface*>(tiled_surface);
    self->_endRequest(request);
}

AutoTiledSurface::AutoTiledSurface() {
    fMyPaintSurface = &fParent.parent;
    mypaint_tiled_surface_init(&fParent,
                               autoTiledSurfaceRequestStart,
                               autoTiledSurfaceRequestEnd);
    fParent.parent.destroy = autoTiledSurfaceFree;
    fParent.threadsafe_tile_requests = false;
}

AutoTiledSurface::~AutoTiledSurface() {
    _free();
}

void AutoTiledSurface::loadBitmap(const SkBitmap& src) {
    mAutoTilesData.loadBitmap(src);
}
#include <omp.h>

void AutoTiledSurface::_startRequest(MyPaintTileRequest * const request) {
    //qDebug() << request->tx << request->ty;
    #pragma omp critical
    {
        mAutoTilesData.stretchToTile(request->tx, request->ty);
        request->buffer = mAutoTilesData.getTile(request->tx, request->ty);
    }
}

void AutoTiledSurface::_endRequest(MyPaintTileRequest * const request) {
    Q_UNUSED(request);
}
