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
    fGlobalBoundingRect = src->fGlobalBoundingRect;
    fTransform = src->fTransform;
    fCustomRelFrame = src->fCustomRelFrame;
    fUseCustomRelFrame = src->fUseCustomRelFrame;
    fRelFrame = src->fRelFrame;
    fRelBoundingRect = src->fRelBoundingRect;
    fRenderTransform = src->fRenderTransform;
    fBlendMode = src->fBlendMode;
    fDrawPos = src->fDrawPos;
    fOpacity = src->fOpacity;
    fResolution = src->fResolution;
    fRenderedImage = SkiaHelpers::makeCopy(src->fRenderedImage);
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
        path.addRect(SkRect::MakeXYWH(fDrawPos.x(), fDrawPos.y(),
                                      fRenderedImage->width(),
                                      fRenderedImage->height()));
        path.toggleInverseFillType();
        canvas->drawPath(path, paintT);
    }
    canvas->drawImage(fRenderedImage, fDrawPos.x(), fDrawPos.y(), &paint);
}

void BoundingBoxRenderData::processTask() {
    updateGlobalFromRelBoundingRect();
    if(fOpacity < 0.001) return;

    const auto info = SkiaHelpers::getPremulBGRAInfo(
                qCeil(fGlobalBoundingRect.width()),
                qCeil(fGlobalBoundingRect.height()));
    fBitmapTMP.allocPixels(info);
    fBitmapTMP.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(fBitmapTMP);
    transformRenderCanvas(canvas);

    drawSk(&canvas);

    for(const auto& effect : fRasterEffects) {
        effect->applyEffectsSk(fBitmapTMP, fResolution);
    }
    clearPixmapEffects();

    fRenderedImage = SkiaHelpers::transferDataToSkImage(fBitmapTMP);
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
        fMotionBlurTarget->fOtherGlobalRects << fGlobalBoundingRect;
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

void BoundingBoxRenderData::updateGlobalFromRelBoundingRect() {
    fResolutionScale.reset();
    fResolutionScale.scale(fResolution, fResolution);
    fScaledTransform = fTransform*fResolutionScale;
    fGlobalBoundingRect = fScaledTransform.mapRect(fRelBoundingRect);
    for(const QRectF &rectT : fOtherGlobalRects) {
        fGlobalBoundingRect = fGlobalBoundingRect.united(rectT);
    }
    fGlobalBoundingRect.adjust(-fEffectsMargin, -fEffectsMargin,
                               fEffectsMargin, fEffectsMargin);
    if(fMaxBoundsEnabled) {
        const auto maxBounds = fResolutionScale.mapRect(fMaxBoundsRect);
        fGlobalBoundingRect = fGlobalBoundingRect.intersected(maxBounds);
    }
    fixupGlobalBoundingRect();
}

void BoundingBoxRenderData::fixupGlobalBoundingRect() {
    const QPointF roundTL(qRound(fGlobalBoundingRect.left()),
                          qRound(fGlobalBoundingRect.top()));
    const QPointF transF = fGlobalBoundingRect.topLeft() - roundTL;
    fGlobalBoundingRect.translate(-transF);
    fDrawPos = {qRound(fGlobalBoundingRect.left()),
                qRound(fGlobalBoundingRect.top())};
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
