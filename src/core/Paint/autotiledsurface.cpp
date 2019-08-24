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
#ifdef _OPENMP
    fParent.threadsafe_tile_requests = true;
#else
    fParent.threadsafe_tile_requests = false;
#endif
}

AutoTiledSurface::~AutoTiledSurface() {
    _free();
}

void AutoTiledSurface::setPixelClamp(const QRect &pixRect) {
    mAutoTilesData.setPixelClamp(pixRect);
}

void AutoTiledSurface::loadBitmap(const SkBitmap& src) {
    mAutoTilesData.loadBitmap(src);
}

#ifdef _OPENMP
#include <omp.h>
#endif

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
