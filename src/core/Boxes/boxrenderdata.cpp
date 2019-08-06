#include "boxrenderdata.h"
#include "boundingbox.h"
#include "skia/skiahelpers.h"

BoxRenderData::BoxRenderData(BoundingBox *parentBoxT) {
    fParentBox = parentBoxT;
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
    const float invScale = toSkScalar(1/fResolution);
    canvas->scale(invScale, invScale);
    canvas->concat(toSkMatrix(fRenderTransform));
    SkPaint paint;
    paint.setAlpha(static_cast<U8CPU>(qRound(fOpacity*2.55)));
    paint.setBlendMode(fBlendMode);
    //paint.setAntiAlias(true);
    //paint.setFilterQuality(kHigh_SkFilterQuality);
    if(fBlendMode == SkBlendMode::kDstIn ||
       fBlendMode == SkBlendMode::kSrcIn ||
       fBlendMode == SkBlendMode::kDstATop) {
        SkPaint paintT;
        paintT.setBlendMode(fBlendMode);
        paintT.setColor(SK_ColorTRANSPARENT);
        SkPath path;
        path.addRect(SkRect::MakeXYWH(fGlobalRect.x(), fGlobalRect.y(),
                                      fRenderedImage->width(),
                                      fRenderedImage->height()));
        path.toggleInverseFillType();
        canvas->drawPath(path, paintT);
    }
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
    if(fParentBox) fParentBox->setupRenderData(fRelFrame, this);
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
    if(fParentBox) fParentBox->scheduleTask(ref<BoxRenderData>());
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
