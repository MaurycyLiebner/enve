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

#ifndef PAINTTARGET_H
#define PAINTTARGET_H
class Canvas;
#include "skia/skiaincludes.h"
#include "simplebrushwrapper.h"
#include "animatedsurface.h"
#include "Boxes/paintbox.h"
#include "onionskin.h"
#include "CacheHandlers/usepointer.h"

struct PaintTarget {
    PaintTarget(Canvas* const canvas) : mCanvas(canvas) {}

    bool needsProcessing() const { return true; }

    void draw(SkCanvas * const canvas,
              const QMatrix& viewTrans,
              const QRect& drawRect,
              const SkFilterQuality filter,
              const bool drawOnion);

    void paintPress(const QPointF& pos,
                    const ulong ts, const qreal pressure,
                    const qreal xTilt, const qreal yTilt,
                    const SimpleBrushWrapper * const brush);
    void paintMove(const QPointF& pos,
                   const ulong ts, const qreal pressure,
                   const qreal xTilt, const qreal yTilt,
                   const SimpleBrushWrapper * const brush);
    void paintRelease();

    void newEmptyFrame() {
        if(!isValid()) return;
        mPaintAnimSurface->newEmptyFrame();
    }

    void setupOnionSkin() {
        if(!isValid()) return;
        qptr<Canvas> canvasP = mCanvas;
        stdsptr<bool> counter = std::make_shared<bool>(true);
        const auto missingLoaded = [canvasP, counter, this]() {
            if(!counter.unique() || !canvasP) return;
            setupOnionSkin();
        };
        mPaintAnimSurface->setupOnionSkinFor(20, mPaintOnion, missingLoaded);
    }

    void setPaintDrawable(DrawableAutoTiledSurface * const surf,
                          const int frame = 0);
    void setPaintBox(PaintBox * const box);

    bool isValid() const {
        return mPaintAnimSurface;
    }

    QRect pixelBoundingRect() const {
        if(!isValid()) return QRect();
        return mPaintDrawable->pixelBoundingRect();
    }
private:
    QRect mTotalRoi;
    ulong mLastTs;
    int mLastFrame = 0;
    qptr<PaintBox> mPaintDrawableBox;
    qptr<AnimatedSurface> mPaintAnimSurface;
    OnionSkin mPaintOnion;
    bool mPaintPressedSinceUpdate = false;
    UsePointer<DrawableAutoTiledSurface> mPaintDrawable;
    bool mChanged = false;
    Canvas * const mCanvas;
};

#endif // PAINTTARGET_H
