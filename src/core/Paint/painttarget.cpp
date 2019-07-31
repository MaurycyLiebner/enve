#include "painttarget.h"
#include "canvas.h"

void PaintTarget::draw(SkCanvas * const canvas,
                       const QMatrix& viewTrans,
                       const QRect& drawRect) {
    const auto canvasRect = viewTrans.inverted().mapRect(drawRect);
    const auto pDrawTrans = mPaintDrawableBox->getTotalTransform();
    const auto relDRect = pDrawTrans.inverted().mapRect(canvasRect);
    canvas->concat(toSkMatrix(pDrawTrans));
    mPaintOnion.draw(canvas);
    mPaintDrawable->drawOnCanvas(canvas, {0, 0}, &relDRect, nullptr);
}

void PaintTarget::setPaintDrawable(DrawableAutoTiledSurface * const surf) {
    mPaintDrawable = surf;
    mPaintPressedSinceUpdate = false;
    mPaintAnimSurface->setupOnionSkinFor(20, mPaintOnion);
}

void PaintTarget::setPaintBox(PaintBox * const box) {
    if(box == mPaintDrawableBox) return;
    if(mPaintDrawableBox) {
        mPaintDrawableBox->showAfterPainting();
        QObject::disconnect(mPaintAnimSurface,
                            &AnimatedSurface::currentSurfaceChanged,
                            mCanvas, nullptr);
        afterPaintAnimSurfaceChanged();
    }
    mPaintDrawableBox = box;
    if(mPaintDrawableBox) {
        mPaintDrawableBox->hideForPainting();
        mPaintAnimSurface = mPaintDrawableBox->getSurface();
        const auto setter = [this](DrawableAutoTiledSurface * const surf) {
            setPaintDrawable(surf);
        };
        QObject::connect(mPaintAnimSurface,
                         &AnimatedSurface::currentSurfaceChanged,
                         mCanvas, setter);
        setPaintDrawable(mPaintAnimSurface->getCurrentSurface());
    } else {
        mPaintAnimSurface = nullptr;
        mPaintDrawable = nullptr;
    }
}

void PaintTarget::paintPress(const QPointF& pos,
                             const ulong ts, const qreal pressure,
                             const qreal xTilt, const qreal yTilt,
                             const SimpleBrushWrapper * const brush) {
    if(mPaintAnimSurface) {
        if(mPaintAnimSurface->anim_isRecording() &&
           !mPaintAnimSurface->anim_getKeyOnCurrentFrame())
            mPaintAnimSurface->anim_saveCurrentValueAsKey();
    }

    if(mPaintDrawable && brush) {
        mPaintPressedSinceUpdate = true;
        const auto& target = mPaintDrawable->surface();
        const auto pDrawTrans = mPaintDrawableBox->getTotalTransform();
        const auto drawPos = pDrawTrans.inverted().map(pos);
        const auto roi =
                target.paintPressEvent(brush->getBrush(),
                                       drawPos, 1, pressure, xTilt, yTilt);
        const QRect qRoi(roi.x, roi.y, roi.width, roi.height);
        mPaintDrawable->pixelRectChanged(qRoi);
        mLastTs = ts;
    }
}

void PaintTarget::paintMove(const QPointF& pos,
                            const ulong ts, const qreal pressure,
                            const qreal xTilt, const qreal yTilt,
                            const SimpleBrushWrapper * const brush) {
    if(mPaintDrawable && brush) {
        const auto& target = mPaintDrawable->surface();
        const double dt = (ts - mLastTs);
        const auto pDrawTrans = mPaintDrawableBox->getTotalTransform();
        const auto drawPos = pDrawTrans.inverted().map(pos);
        const auto roi =
                target.paintMoveEvent(brush->getBrush(),
                                      drawPos, dt/1000, pressure,
                                      xTilt, yTilt);
        const QRect qRoi(roi.x, roi.y, roi.width, roi.height);
        mPaintDrawable->pixelRectChanged(qRoi);
    }
    mLastTs = ts;
}
