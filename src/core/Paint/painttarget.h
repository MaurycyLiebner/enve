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

#ifndef PAINTTARGET_H
#define PAINTTARGET_H
class Canvas;
#include "skia/skiaincludes.h"
#include "simplebrushwrapper.h"
#include "animatedsurface.h"
#include "Boxes/paintbox.h"

struct PaintTarget {
    PaintTarget(Canvas* const canvas) : mCanvas(canvas) {}

    bool needsProcessing() const { return true; }

    void draw(SkCanvas * const canvas,
              const QMatrix& viewTrans,
              const QRect& drawRect);

    void paintPress(const QPointF& pos,
                    const ulong ts, const qreal pressure,
                    const qreal xTilt, const qreal yTilt,
                    const SimpleBrushWrapper * const brush);
    void paintMove(const QPointF& pos,
                   const ulong ts, const qreal pressure,
                   const qreal xTilt, const qreal yTilt,
                   const SimpleBrushWrapper * const brush);

    void newEmptyFrame() {
        if(!isValid()) return;
        mPaintAnimSurface->newEmptyFrame();
    }

    void setupOnionSkin() {
        if(!isValid()) return;
        mPaintAnimSurface->setupOnionSkinFor(20, mPaintOnion);
    }

    void afterPaintAnimSurfaceChanged();
    void setPaintDrawable(DrawableAutoTiledSurface * const surf);
    void setPaintBox(PaintBox * const box);

    bool isValid() const {
        return mPaintAnimSurface;
    }

    QRect pixelBoundingRect() const {
        if(!isValid()) return QRect();
        return mPaintDrawable->pixelBoundingRect();
    }

    ulong mLastTs;
    qptr<PaintBox> mPaintDrawableBox;
    qptr<AnimatedSurface> mPaintAnimSurface;
    AnimatedSurface::OnionSkin mPaintOnion;
    bool mPaintPressedSinceUpdate = false;
    DrawableAutoTiledSurface * mPaintDrawable = nullptr;
    Canvas * const mCanvas;
};

#endif // PAINTTARGET_H
