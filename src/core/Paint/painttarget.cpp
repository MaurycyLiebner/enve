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

#include "painttarget.h"
#include "canvas.h"
#include "Private/document.h"

void PaintTarget::draw(SkCanvas * const canvas,
                       const QMatrix& viewTrans,
                       const QRect& drawRect,
                       const SkFilterQuality filter,
                       const bool drawOnion) {
    if(!isValid()) return;
    const auto canvasRect = viewTrans.inverted().mapRect(drawRect);
    const auto pDrawTrans = mPaintDrawableBox->getTotalTransform();
    const auto relDRect = pDrawTrans.inverted().mapRect(canvasRect);
    canvas->concat(toSkMatrix(pDrawTrans));
    if(drawOnion) mPaintOnion.draw(canvas);
    SkPaint paint;
    paint.setFilterQuality(filter);
    mPaintDrawable->drawOnCanvas(canvas, mRelDrawPos, &relDRect, &paint);
}

void PaintTarget::setPaintDrawable(DrawableAutoTiledSurface * const surf,
                                   const int frame) {
    if(mPaintDrawable) {
        if(mChanged) {
            mPaintDrawable->drawingDoneForNow();
            if(mPaintAnimSurface) {
                const auto updateRange =
                        mPaintAnimSurface->prp_getIdenticalRelRange(mLastFrame);
                mPaintAnimSurface->prp_afterChangedRelRange(updateRange);
            }
        }
    }
    mPaintDrawable = surf;
    mLastFrame = frame;
    if(mPaintDrawable) {
        if(mPaintDrawable->storesDataInMemory()) {
            if(!mPaintDrawable->hasTileBitmaps())
                mPaintDrawable->updateTileBitmaps();
        } else mPaintDrawable->scheduleLoadFromTmpFile();
    }
    mChanged = false;
    setupOnionSkin();
}

void PaintTarget::setPaintBox(PaintBox * const box) {
    if(box == mPaintDrawableBox) return;
    if(mPaintDrawableBox) {
        mPaintDrawableBox->setVisibleForScene(true);
        QObject::disconnect(mPaintAnimSurface,
                            &AnimatedSurface::currentSurfaceChanged,
                            mCanvas, nullptr);
    }
    mPaintDrawableBox = box;
    if(mPaintDrawableBox) {
        mPaintDrawableBox->setVisibleForScene(false);
        mPaintAnimSurface = mPaintDrawableBox->getSurface();
        const auto setter = [this](DrawableAutoTiledSurface * const surf) {
            setPaintDrawable(surf, mPaintAnimSurface->anim_getCurrentRelFrame());
        };
        QObject::connect(mPaintAnimSurface,
                         &AnimatedSurface::currentSurfaceChanged,
                         mCanvas, setter);
        setPaintDrawable(mPaintAnimSurface->getCurrentSurface(),
                         mPaintAnimSurface->anim_getCurrentRelFrame());
    } else {
        setPaintDrawable(nullptr);
        mPaintAnimSurface = nullptr;
    }
}

QPointF PaintTarget::absPosToRelPos(const QPointF& absPos) const {
    if(mPaintDrawable) {
        const auto pDrawTrans = mPaintDrawableBox->getTotalTransform();
        return pDrawTrans.inverted().map(absPos);
    }
    return absPos;
}

void PaintTarget::startTransform() {
    if(mPaintAnimSurface) {
        if(mPaintAnimSurface->anim_isRecording() &&
           !mPaintAnimSurface->anim_getKeyOnCurrentFrame())
            mPaintAnimSurface->anim_saveCurrentValueAsKey();
    }
}


void PaintTarget::cropPress(const QPointF &pos) {
    Q_UNUSED(pos)
}

void PaintTarget::cropMove(const QPointF &pos) {
    Q_UNUSED(pos)
}

void PaintTarget::cropRelease(const QPointF &pos) {
    startTransform();
    Q_UNUSED(pos)
}

void PaintTarget::cropCancel() {
    mRelDrawPos = {0, 0};
}

void PaintTarget::movePress(const QPointF &pos) {
    mMovePress = absPosToRelPos(pos);
}

void PaintTarget::moveMove(const QPointF &pos) {
    startTransform();
    mRelDrawPos = toSkPoint(absPosToRelPos(pos) - mMovePress);
}

void PaintTarget::moveRelease(const QPointF &pos) {
    moveMove(pos);
    const int dx = qRound(mRelDrawPos.x());
    const int dy = qRound(mRelDrawPos.y());
    mChanged = true;
    if(mPaintDrawable) {
        mPaintDrawable->move(dx, dy);
        if(mPaintAnimSurface) {
            mPaintAnimSurface->prp_pushUndoRedoName("Move");
            UndoRedo ur;
            const auto anim = mPaintAnimSurface;
            const stdptr<DrawableAutoTiledSurface> ptr = mPaintDrawable.get();
            ur.fUndo = [dx, dy, ptr]() {
                if(!ptr) return;
                ptr->move(-dx, -dy);
            };
            ur.fRedo = [dx, dy, ptr]() {
                if(!ptr) return;
                ptr->move(dx, dy);
            };
            mPaintAnimSurface->prp_addUndoRedo(ur);
        }
        Document::sInstance->actionFinished();
    }
    mRelDrawPos = {0, 0};
}

void PaintTarget::moveCancel() {
    mRelDrawPos = {0, 0};
}

void PaintTarget::paintPress(const QPointF& pos,
                             const ulong ts, const qreal pressure,
                             const qreal xTilt, const qreal yTilt,
                             const SimpleBrushWrapper * const brush) {
    startTransform();

    if(mPaintDrawable && brush) {
        const auto& target = mPaintDrawable->surface();
        const auto pDrawTrans = mPaintDrawableBox->getTotalTransform();
        const auto drawPos = pDrawTrans.inverted().map(pos);
        const auto roi =
                target.paintPressEvent(brush->getBrush(),
                                       drawPos, 1, pressure, xTilt, yTilt);
        const QRect qRoi(roi.x, roi.y, roi.width, roi.height);
        mTotalRoi = qRoi;
        mPaintDrawable->pixelRectChanged(qRoi);
        mLastTs = ts;
        mChanged = true;
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
        if(mTotalRoi.isNull()) {
            mTotalRoi = qRoi;
        } else {
            mTotalRoi = mTotalRoi.united(qRoi);
        }
    }
    mLastTs = ts;
}

void PaintTarget::paintRelease() {
    if(mPaintAnimSurface && mPaintDrawable) {
        auto& target = mPaintDrawable->surface();
        auto undoList = target.takeUndoList();
        if(undoList.isEmpty()) return;
        {
            mPaintAnimSurface->prp_pushUndoRedoName("Paint");
            const stdptr<DrawableAutoTiledSurface> ptr = mPaintDrawable.get();
            const QRect roi = mTotalRoi;
            UndoRedo ur;
            const auto anim = mPaintAnimSurface;

            const auto replaceTile = [undoList, anim, ptr, roi](
                    const stdsptr<Tile>& (UndoTile::*getter)() const) {
                if(!ptr) return;
                auto& surface = ptr->surface();
                for(const auto& undoTile : undoList) {
                    surface.replaceTile(undoTile.tileX(),
                                        undoTile.tileY(),
                                        (undoTile.*getter)());
                }
                surface.autoCrop();
                ptr->updateTileDimensions();
                ptr->pixelRectChanged(roi);
                const int relFrame = anim->anim_getCurrentRelFrame();
                const auto identicalRange = anim->prp_getIdenticalRelRange(relFrame);
                anim->prp_afterChangedRelRange(identicalRange);
            };

            ur.fUndo = [replaceTile]() {
                replaceTile(&UndoTile::oldValue);
            };
            ur.fRedo = [replaceTile]() {
                replaceTile(&UndoTile::newValue);
            };
            mPaintAnimSurface->prp_addUndoRedo(ur);
        }
        Document::sInstance->actionFinished();
    }
}
