#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "colorconversions.h"
#include "autotiledsurface.h"

bool gRead(QIODevice *src, AutoTiledSurface &value) {
    return true;
}

bool gWrite(QIODevice *dst, const AutoTiledSurface &value) {
    return true;
}

bool gDiffers(const AutoTiledSurface &surf1, const AutoTiledSurface &surf2) {
    return &surf1 != &surf2;
}

void gInterpolate(const AutoTiledSurface &surf1, const AutoTiledSurface &surf2,
                  const qreal &surf2Weight, AutoTiledSurface &target)  {

}

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
}

AutoTiledSurface::~AutoTiledSurface() {
    _free();
}

void AutoTiledSurface::loadBitmap(const SkBitmap& src) {
    mAutoTilesData.loadBitmap(src);
}

void AutoTiledSurface::_startRequest(MyPaintTileRequest * const request) {
    //qDebug() << request->tx << request->ty;
    mAutoTilesData.stretchToTile(request->tx, request->ty);
    request->buffer = mAutoTilesData.getTile(
                request->tx, request->ty);
}

void AutoTiledSurface::_endRequest(MyPaintTileRequest * const request) {
    Q_UNUSED(request);
}
