#include "boxrenderdata.h"
#include "boundingbox.h"
#include "skia/skiahelpers.h"
#include "PixmapEffects/pixmapeffect.h"

BoxRenderData::BoxRenderData(BoundingBox *parentBoxT) {
    fParentBox = parentBoxT;
}

void BoxRenderData::transformRenderCanvas(SkCanvas &canvas) const {
    canvas.translate(toSkScalar(-fGlobalRect.x()),
                     toSkScalar(-fGlobalRect.y()));
    canvas.concat(toSkMatrix(fScaledTransform));
}

void BoxRenderData::copyFrom(BoxRenderData *src) {
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

stdsptr<BoxRenderData> BoxRenderData::makeCopy() {
    if(!fParentBox) return nullptr;
    stdsptr<BoxRenderData> copy = fParentBox->createRenderData();
    copy->copyFrom(this);
    return copy;
}

void BoxRenderData::updateRelBoundingRect() {
    if(!fParentBox) return;
    fRelBoundingRect = fParentBox->getRelBoundingRect(fRelFrame);
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

void BoxRenderData::processGPU(QGL33 * const gl,
                               SwitchableContext &context) {
    if(mStep == Step::EFFECTS)
        return mEffectsRenderer.processGpu(gl, context, this);
    updateGlobalRect();
    if(fOpacity < 0.001) return;
    if(fGlobalRect.width() <= 0 || fGlobalRect.height() <= 0) return;

    context.switchToSkia();
    const auto grContext = context.requestContext();

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
    if(mEffectsRenderer.isEmpty() ||
       mEffectsRenderer.nextHardwareSupport() == HardwareSupport::CPU_ONLY)
        fRenderedImage = fRenderedImage->makeRasterImage();
    else mEffectsRenderer.processGpu(gl, context, this);
//    GrGLTextureInfo info;
//    grTex.getGLTextureInfo(&info);
//    info.fID;
//    fRenderedImage = tex.toImage(gl);
//    tex.clear(gl);
}

void BoxRenderData::process() {
    if(mStep == Step::EFFECTS)
        return mEffectsRenderer.processCpu(this);
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

void BoxRenderData::beforeProcessing() {
    setupRenderData();
    if(!mDataSet) dataSet();

    if(!fParentBox || !fParentIsTarget) return;
    if(nullifyBeforeProcessing())
        fParentBox->nullifyCurrentRenderData(fRelFrame);
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
}

HardwareSupport BoxRenderData::hardwareSupport() const {
    if(mStep == Step::EFFECTS) {
        return mEffectsRenderer.nextHardwareSupport();
    } else {
        if(fParentBox && fParentBox->SWT_isLayerBox())
            return HardwareSupport::GPU_PREFFERED;
        return HardwareSupport::CPU_PREFFERED;
    }
}

void BoxRenderData::scheduleTaskNow() {
    if(fParentBox) fParentBox->scheduleTask(ref<BoxRenderData>());
}

void BoxRenderData::afterCanceled() {
    if(fRefInParent && fParentBox)
        fParentBox->nullifyCurrentRenderData(fRelFrame);
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

bool BoxRenderData::nullifyBeforeProcessing() {
    return fReason == BoundingBox::FRAME_CHANGE;
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

RenderDataCustomizerFunctor::RenderDataCustomizerFunctor() {}

void RenderDataCustomizerFunctor::operator()(BoxRenderData * const data) {
    customize(data);
}

ReplaceTransformDisplacementCustomizer::ReplaceTransformDisplacementCustomizer(
        const qreal dx, const qreal dy) {
    mDx = dx;
    mDy = dy;
}

void ReplaceTransformDisplacementCustomizer::customize(
        BoxRenderData * const data) {
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

void MultiplyTransformCustomizer::customize(BoxRenderData * const data) {
    data->fTransform = mTransform*data->fTransform;
    data->fOpacity *= mOpacity;
}

MultiplyOpacityCustomizer::MultiplyOpacityCustomizer(const qreal opacity) {
    mOpacity = opacity;
}

void MultiplyOpacityCustomizer::customize(BoxRenderData * const data) {
    data->fOpacity *= mOpacity;
}
