#ifndef PAINTTARGET_H
#define PAINTTARGET_H
class Canvas;
#include "skia/skiaincludes.h"
#include "GUI/BrushWidgets/simplebrushwrapper.h"
#include "animatedsurface.h"
#include "Boxes/paintbox.h"

struct PaintTarget {
    PaintTarget(Canvas* const canvas) : mCanvas(canvas) {}

    bool needsProcessing() const { return true; }

    void draw(SkCanvas * const canvas,
              GrContext * const grContext,
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
        mPaintAnimSurface->newEmptyFrame();
    }

    void setupOnionSkin() {
        mPaintAnimSurface->setupOnionSkinFor(20, mPaintOnion);
    }

    void afterPaintAnimSurfaceChanged();
    void setPaintDrawable(DrawableAutoTiledSurface * const surf);
    void setPaintBox(PaintBox * const box);

    bool isValid() const {
        return mPaintAnimSurface;
    }

    QRect pixelBoundingRect() const {
        return mPaintDrawable->pixelBoundingRect();
    }

    ulong mLastTs;
    qptr<PaintBox> mPaintDrawableBox;
    qptr<AnimatedSurface> mPaintAnimSurface;
    AnimatedSurface::OnionSkin mPaintOnion;
    bool mPaintPressedSinceUpdate = false;
    DrawableAutoTiledSurface * mPaintDrawable = nullptr;
    Canvas * mCanvas = nullptr;
};

#endif // PAINTTARGET_H
