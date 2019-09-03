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

#include "boxrenderdata.h"
#include "boundingbox.h"
#include "skia/skiahelpers.h"
#include "efiltersettings.h"
#include "Tasks/taskscheduler.h"

BoxRenderData::BoxRenderData(BoundingBox * const parent) :
    fFilterQuality(eFilterSettings::sRender()) {
    fParentBox = parent;
}

void BoxRenderData::transformRenderCanvas(SkCanvas &canvas) const {
    canvas.translate(toSkScalar(-fGlobalRect.x()),
                     toSkScalar(-fGlobalRect.y()));
    canvas.concat(toSkMatrix(fScaledTransform));
}

void BoxRenderData::copyFrom(BoxRenderData *src) {
    fRelTransform = src->fRelTransform;
    fTransform = src->fTransform;
    fRelFrame = src->fRelFrame;
    fRelBoundingRect = src->fRelBoundingRect;
    fRenderTransform = src->fRenderTransform;
    fAntiAlias = src->fAntiAlias;
    fUseRenderTransform = src->fUseRenderTransform;
    fBlendMode = src->fBlendMode;
    fGlobalRect = src->fGlobalRect;
    fOpacity = src->fOpacity;
    fResolution = src->fResolution;
    fRenderedImage = SkiaHelpers::makeCopy(src->fRenderedImage);
    fBoxStateId = src->fBoxStateId;
    mState = eTaskState::finished;
    fRelBoundingRectSet = true;
}

stdsptr<BoxRenderData> BoxRenderData::makeCopy() {
    if(!fParentBox) return nullptr;
    stdsptr<BoxRenderData> copy = fParentBox->createRenderData();
    copy->copyFrom(this);
    return copy;
}

void BoxRenderData::drawRenderedImageForParent(SkCanvas * const canvas) {
    if(fOpacity < 0.001) return;
    if(fUseRenderTransform) canvas->concat(toSkMatrix(fRenderTransform));
    if(fBlendMode == SkBlendMode::kDstIn ||
       fBlendMode == SkBlendMode::kSrcIn ||
       fBlendMode == SkBlendMode::kDstATop) {
        SkPaint bPaint;
        bPaint.setBlendMode(fBlendMode);
        bPaint.setColor(SK_ColorTRANSPARENT);
        SkPath path;
        path.addRect(SkRect::MakeXYWH(fGlobalRect.x(), fGlobalRect.y(),
                                      fRenderedImage->width(),
                                      fRenderedImage->height()));
        path.toggleInverseFillType();
        canvas->drawPath(path, bPaint);
    }
    SkPaint paint;
    paint.setAlpha(static_cast<U8CPU>(qRound(fOpacity*2.55)));
    paint.setBlendMode(fBlendMode);
    paint.setAntiAlias(fAntiAlias);
    if(fUseRenderTransform) paint.setFilterQuality(fFilterQuality);
    canvas->drawImage(fRenderedImage, fGlobalRect.x(), fGlobalRect.y(), &paint);
}

void BoxRenderData::processGpu(QGL33 * const gl,
                               SwitchableContext &context) {
    if(mStep == Step::EFFECTS)
        return mEffectsRenderer.processGpu(gl, context, this);
    updateGlobalRect();
    if(fOpacity < 0.001) return;
    if(fGlobalRect.width() <= 0 || fGlobalRect.height() <= 0) return;

    context.switchToSkia();
    const auto grContext = context.requestContext();

    const auto grTex = grContext->createBackendTexture(
                fGlobalRect.width(), fGlobalRect.height(),
                kRGBA_8888_SkColorType, GrMipMapped::kNo,
                GrRenderable::kYes);
    const auto surf = SkSurface::MakeFromBackendTexture(
                grContext, grTex, kTopLeft_GrSurfaceOrigin, 0,
                kRGBA_8888_SkColorType, nullptr, nullptr);

    const auto canvas = surf->getCanvas();
    transformRenderCanvas(*canvas);
    canvas->clear(eraseColor());
    drawSk(canvas);
    canvas->flush();
    fRenderedImage = SkImage::MakeFromAdoptedTexture(grContext, grTex,
                                                     kTopLeft_GrSurfaceOrigin,
                                                     kRGBA_8888_SkColorType);
    if(mEffectsRenderer.isEmpty() ||
       mEffectsRenderer.nextHardwareSupport() == HardwareSupport::cpuOnly)
        fRenderedImage = fRenderedImage->makeRasterImage();
    else mEffectsRenderer.processGpu(gl, context, this);
}

void BoxRenderData::process() {
    if(mStep == Step::EFFECTS) return;
    updateGlobalRect();
    if(fOpacity < 0.001) return;
    if(fGlobalRect.width() <= 0 || fGlobalRect.height() <= 0) return;

    SkBitmap bitmap;
    const auto info = SkiaHelpers::getPremulRGBAInfo(fGlobalRect.width(),
                                                     fGlobalRect.height());
    bitmap.allocPixels(info);
    bitmap.eraseColor(eraseColor());
    SkCanvas canvas(bitmap);
    transformRenderCanvas(canvas);

    drawSk(&canvas);

    fRenderedImage = SkiaHelpers::transferDataToSkImage(bitmap);
}

void BoxRenderData::beforeProcessing(const Hardware hw) {
    if(mStep == Step::EFFECTS) {
        if(hw == Hardware::cpu) {
            mState = eTaskState::waiting;
            mEffectsRenderer.processCpu(this);
        }
        return;
    }
    setupRenderData();
    if(!mDataSet) dataSet();
}

void BoxRenderData::afterProcessing() {
    if(fMotionBlurTarget) {
        fMotionBlurTarget->fOtherGlobalRects << fGlobalRect;
    }
    if(fParentBox && fParentIsTarget) {
        fParentBox->renderDataFinished(this);
    }
}

void BoxRenderData::afterQued() {
    if(mDataSet) return;
    if(!mDelayDataSet) dataSet();
}

HardwareSupport BoxRenderData::hardwareSupport() const {
    if(mStep == Step::EFFECTS) {
        return mEffectsRenderer.nextHardwareSupport();
    } else {
        if(fParentBox && fParentBox->SWT_isLayerBox())
            return HardwareSupport::gpuPreffered;
        return HardwareSupport::cpuPreffered;
    }
}

void BoxRenderData::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleCpuTask(ref<eTask>());
}

void BoxRenderData::dataSet() {
    if(mDataSet) return;
    mDataSet = true;
    if(!fRelBoundingRectSet) {
        fRelBoundingRectSet = true;
        updateRelBoundingRect();
    }
    if(!fParentBox || !fParentIsTarget) return;
    fParentBox->updateCurrentPreviewDataFromRenderData(this);
}

void BoxRenderData::updateGlobalRect() {
    fResolutionScale.reset();
    fResolutionScale.scale(fResolution, fResolution);
    fScaledTransform = fTransform*fResolutionScale;
    QRectF baseRectF = fScaledTransform.mapRect(fRelBoundingRect);
    for(const QRectF &rectT : fOtherGlobalRects) {
        baseRectF = baseRectF.united(rectT);
    }
    baseRectF.adjust(-fBaseMargin.left(), -fBaseMargin.top(),
                     fBaseMargin.right(), fBaseMargin.bottom());
    setBaseGlobalRect(baseRectF);
}

void BoxRenderData::setBaseGlobalRect(const QRectF& baseRectF) {
    const QRectF maxBounds = fResolutionScale.mapRect(QRectF(fMaxBoundsRect));
    const auto clampedBaseRect = baseRectF.intersected(maxBounds);
    SkIRect currRect = toSkRect(clampedBaseRect).roundOut();
    if(!mEffectsRenderer.isEmpty()) {
        const QRect iMaxBounds(qFloor(maxBounds.left()), qFloor(maxBounds.top()),
                               qCeil(maxBounds.width()), qCeil(maxBounds.height()));
        const SkIRect skMaxBounds = toSkIRect(iMaxBounds);
        mEffectsRenderer.setBaseGlobalRect(currRect, skMaxBounds);
    }
    fGlobalRect = toQRect(currRect);
}
