#include "boundingboxrenderdata.h"
#include "boundingbox.h"
#include "PixmapEffects/rastereffects.h"
#include "GPUEffects/gpupostprocessor.h"
#include "skia/skiahelpers.h"
#include "PixmapEffects/pixmapeffect.h"

BoundingBoxRenderData::BoundingBoxRenderData(BoundingBox *parentBoxT) {
    fParentBox = parentBoxT;
}

void BoundingBoxRenderData::copyFrom(BoundingBoxRenderData *src) {
    fTransform = src->fTransform;
    fCustomRelFrame = src->fCustomRelFrame;
    fUseCustomRelFrame = src->fUseCustomRelFrame;
    fRelFrame = src->fRelFrame;
    fRelBoundingRect = src->fRelBoundingRect;
    fRenderTransform = src->fRenderTransform;
    fBlendMode = src->fBlendMode;
    fGlobalRect = src->fGlobalRect;
    fOpacity = src->fOpacity;
    fResolution = src->fResolution;
    fRenderedImage = SkiaHelpers::makeCopy(src->fRenderedImage);
    fBoxStateId = src->fBoxStateId;
    mState = FINISHED;
    fRelBoundingRectSet = true;
    fCopied = true;
}

stdsptr<BoundingBoxRenderData> BoundingBoxRenderData::makeCopy() {
    if(!fParentBox) return nullptr;
    stdsptr<BoundingBoxRenderData> copy = fParentBox->createRenderData();
    copy->copyFrom(this);
    return copy;
}

void BoundingBoxRenderData::updateRelBoundingRect() {
    if(!fParentBox) return;
    fRelBoundingRect = fParentBox->getRelBoundingRect(fRelFrame);
}

void BoundingBoxRenderData::drawRenderedImageForParent(SkCanvas * const canvas) {
    if(fOpacity < 0.001) return;
    const SkScalar invScale = toSkScalar(1/fResolution);
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

void BoundingBoxRenderData::processTaskWithGPU(QGL33 * const gl,
                                               GrContext * const grContext) {
    Q_UNUSED(gl);
    updateGlobalRect();
    if(fOpacity < 0.001) return;
    if(fGlobalRect.width() <= 0 || fGlobalRect.height() <= 0) return;

    //const auto info = SkiaHelpers::getPremulRGBAInfo(width, height);
//    Texture tex;
//    tex.gen(gl, width, height, nullptr);
//    GrGLTextureInfo texInfo;
//    texInfo.fID = tex.fId;
//    texInfo.fFormat = GR_GL_RGBA8;
//    texInfo.fTarget = GR_GL_TEXTURE_2D;
    const auto grTex = grContext->createBackendTexture(
                fGlobalRect.width(), fGlobalRect.height(),
                kRGBA_8888_SkColorType, GrMipMapped::kNo,
                GrRenderable::kYes);
//    const auto grTex = GrBackendTexture(tex.fWidth, tex.fHeight,
//                                        GrMipMapped::kNo, texInfo);
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
//    GrGLTextureInfo info;
//    grTex.getGLTextureInfo(&info);
//    info.fID;
//    fRenderedImage = tex.toImage(gl);
//    tex.clear(gl);
}

void BoundingBoxRenderData::processTask() {
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

    for(const auto& effect : fRasterEffects) {
        effect->applyEffectsSk(bitmap, fResolution);
    }
    clearPixmapEffects();

    fRenderedImage = SkiaHelpers::transferDataToSkImage(bitmap);
}

void BoundingBoxRenderData::beforeProcessing() {
    setupRenderData();
    if(!mDataSet) dataSet();

    if(!fParentBox || !fParentIsTarget) return;
    if(nullifyBeforeProcessing())
        fParentBox->nullifyCurrentRenderData(fRelFrame);
}

void BoundingBoxRenderData::afterProcessing() {
    if(fMotionBlurTarget) {
        fMotionBlurTarget->fOtherGlobalRects << fGlobalRect;
    }
    if(fParentBox && fParentIsTarget) {
        fParentBox->renderDataFinished(this);
    }
}

void BoundingBoxRenderData::taskQued() {
    mDataSet = false;
    if(fParentBox) {
        if(fUseCustomRelFrame) {
            fParentBox->setupRenderData(fCustomRelFrame, this);
        } else {
            fParentBox->setupRenderData(fRelFrame, this);
        }
        for(const auto& customizer : mRenderDataCustomizerFunctors) {
            (*customizer)(this);
        }
    }
    if(!mDelayDataSet) dataSet();
    Task::taskQued();
}

Task::GpuSupport BoundingBoxRenderData::gpuSupport() const {
    if(fGPUEffects.isEmpty()) {
        if(fParentBox && fParentBox->SWT_isLayerBox())
            return GPU_PREFERRED;
        return GPU_SUPPORTED;
    }
    return GPU_REQUIRED;
}

void BoundingBoxRenderData::scheduleTaskNow() {
    if(fParentBox) fParentBox->scheduleTask(ref<BoundingBoxRenderData>());
}

void BoundingBoxRenderData::afterCanceled() {
    if(fRefInParent && fParentBox)
        fParentBox->nullifyCurrentRenderData(fRelFrame);
}

void BoundingBoxRenderData::dataSet() {
    if(mDataSet) return;
    mDataSet = true;
    if(!fRelBoundingRectSet) {
        fRelBoundingRectSet = true;
        updateRelBoundingRect();
    }
    if(!fParentBox || !fParentIsTarget) return;
    fParentBox->updateCurrentPreviewDataFromRenderData(this);
}

bool BoundingBoxRenderData::nullifyBeforeProcessing() {
    return fReason == BoundingBox::FRAME_CHANGE;
}

void BoundingBoxRenderData::updateGlobalRect() {
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

void BoundingBoxRenderData::setBaseGlobalRect(const QRectF& baseRectF) {
    const QRectF maxBounds = fResolutionScale.mapRect(QRectF(fMaxBoundsRect));
    const auto clampedBaseRect = baseRectF.intersected(maxBounds);
    SkIRect currRect = toSkRect(clampedBaseRect).roundOut();
    if(!fGPUEffects.isEmpty()) {
        const QRect iMaxBounds(qFloor(maxBounds.left()), qFloor(maxBounds.top()),
                               qCeil(maxBounds.width()), qCeil(maxBounds.height()));
        const SkIRect skMaxBounds = toSkIRect(iMaxBounds);
        for(const auto& effect : fGPUEffects) {
            currRect = effect->setSrcRectUpdateDstRect(currRect, skMaxBounds);
        }
    }
    fGlobalRect = toQRect(currRect);
}

RenderDataCustomizerFunctor::RenderDataCustomizerFunctor() {}

void RenderDataCustomizerFunctor::operator()(BoundingBoxRenderData * const data) {
    customize(data);
}

ReplaceTransformDisplacementCustomizer::ReplaceTransformDisplacementCustomizer(
        const qreal dx, const qreal dy) {
    mDx = dx;
    mDy = dy;
}

void ReplaceTransformDisplacementCustomizer::customize(
        BoundingBoxRenderData * const data) {
    QMatrix transformT = data->fTransform;
    data->fTransform.setMatrix(transformT.m11(), transformT.m12(),
                               transformT.m21(), transformT.m22(),
                               mDx, mDy);
}

MultiplyTransformCustomizer::MultiplyTransformCustomizer(
        const QMatrix &transform, const qreal opacity) {
    mTransform = transform;
    mOpacity = opacity;
}

void MultiplyTransformCustomizer::customize(BoundingBoxRenderData * const data) {
    data->fTransform = mTransform*data->fTransform;
    data->fOpacity *= mOpacity;
}

MultiplyOpacityCustomizer::MultiplyOpacityCustomizer(const qreal opacity) {
    mOpacity = opacity;
}

void MultiplyOpacityCustomizer::customize(BoundingBoxRenderData * const data) {
    data->fOpacity *= mOpacity;
}
