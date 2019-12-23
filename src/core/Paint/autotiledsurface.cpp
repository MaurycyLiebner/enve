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

AutoTiledSurface::AutoTiledSurface() :
    fMyPaintSurface(&fParent.parent) {
    mypaint_tiled_surface_init(&fParent,
                               sRequestStart,
                               sRequestEnd);
    fParent.parent.destroy = sFree;
#ifdef _OPENMP
    fParent.threadsafe_tile_requests = true;
#else
    fParent.threadsafe_tile_requests = false;
#endif
}

AutoTiledSurface::AutoTiledSurface(const AutoTiledSurface &other) :
    AutoTiledSurface() {
    mAutoTilesData = other.mAutoTilesData;
}

AutoTiledSurface::AutoTiledSurface(AutoTiledSurface &&other) :
    AutoTiledSurface() {
    mAutoTilesData = std::move(other.mAutoTilesData);
}

AutoTiledSurface &AutoTiledSurface::operator=(const AutoTiledSurface &other) {
    mAutoTilesData = other.mAutoTilesData;
    return *this;
}

AutoTiledSurface &AutoTiledSurface::operator=(AutoTiledSurface &&other) {
    mAutoTilesData = std::move(other.mAutoTilesData);
    return *this;
}

AutoTiledSurface::~AutoTiledSurface() {
    free();
}

void AutoTiledSurface::setPixelClamp(const QRect &pixRect) {
    mAutoTilesData.setPixelClamp(pixRect);
}

void AutoTiledSurface::loadBitmap(const SkBitmap& src) {
    mAutoTilesData.loadBitmap(src);
}

void AutoTiledSurface::sFree(MyPaintSurface *surface) {
    const auto self = reinterpret_cast<AutoTiledSurface*>(surface);
    self->free();
}

void AutoTiledSurface::sRequestStart(MyPaintTiledSurface *tiled_surface,
                                     MyPaintTileRequest *request) {
    const auto self = reinterpret_cast<AutoTiledSurface*>(tiled_surface);
    self->startRequest(request);
}

void AutoTiledSurface::sRequestEnd(MyPaintTiledSurface *tiled_surface,
                                   MyPaintTileRequest *request) {
    const auto self = reinterpret_cast<AutoTiledSurface*>(tiled_surface);
    self->endRequest(request);
}

void AutoTiledSurface::free() {
    mypaint_tiled_surface_destroy(&fParent);
}

#ifdef _OPENMP
#include <omp.h>
#endif

void AutoTiledSurface::startRequest(MyPaintTileRequest * const request) {
    #pragma omp critical
    {
        mAutoTilesData.stretchToTile(request->tx, request->ty);
        request->buffer = mAutoTilesData.getTile(request->tx, request->ty);
    }
}

void AutoTiledSurface::endRequest(MyPaintTileRequest * const request) {
    Q_UNUSED(request)
}
