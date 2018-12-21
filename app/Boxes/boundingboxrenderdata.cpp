#include "boundingboxrenderdata.h"
#include "boundingbox.h"
#include "PixmapEffects/rastereffects.h"
#include "gpupostprocessor.h"
#include "skimagecopy.h"
#include "PixmapEffects/pixmapeffect.h"

BoundingBoxRenderData::BoundingBoxRenderData(BoundingBox *parentBoxT) {
    if(parentBoxT == nullptr) return;
    fParentBox = parentBoxT;
}

BoundingBoxRenderData::~BoundingBoxRenderData() {}

void BoundingBoxRenderData::copyFrom(BoundingBoxRenderData *src) {
    fGlobalBoundingRect = src->fGlobalBoundingRect;
    fTransform = src->fTransform;
    fParentTransform = src->fParentTransform;
    fCustomRelFrame = src->fCustomRelFrame;
    fUseCustomRelFrame = src->fUseCustomRelFrame;
    fRelFrame = src->fRelFrame;
    fRelBoundingRect = src->fRelBoundingRect;
    fRelTransform = src->fRelTransform;
    fRenderedToImage = src->fRenderedToImage;
    fBlendMode = src->fBlendMode;
    fDrawPos = src->fDrawPos;
    fOpacity = src->fOpacity;
    fResolution = src->fResolution;
    renderedImage = makeSkImageCopy(src->renderedImage);
    mFinished = true;
    fRelBoundingRectSet = true;
    fCopied = true;
}

stdsptr<BoundingBoxRenderData> BoundingBoxRenderData::makeCopy() {
    BoundingBox *parentBoxT = fParentBox.data();
    if(parentBoxT == nullptr) return nullptr;
    stdsptr<BoundingBoxRenderData> copy = parentBoxT->createRenderData();
    copy->copyFrom(this);
    return copy;
}

void BoundingBoxRenderData::updateRelBoundingRect() {
    BoundingBox *parentBoxT = fParentBox.data();
    if(parentBoxT == nullptr) return;
    fRelBoundingRect = parentBoxT->getRelBoundingRectAtRelFrame(fRelFrame);
}

void BoundingBoxRenderData::drawRenderedImageForParent(SkCanvas *canvas) {
    if(fOpacity < 0.001) return;
    canvas->save();
    SkScalar invScale = 1.f/qrealToSkScalar(fResolution);
    canvas->scale(invScale, invScale);
    renderToImage();
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
                                      renderedImage->width(),
                                      renderedImage->height()));
        path.toggleInverseFillType();
        canvas->drawPath(path, paintT);
    }
    canvas->drawImage(renderedImage,
                      fDrawPos.x(), fDrawPos.y(),
                      &paint);
    canvas->restore();
}

void BoundingBoxRenderData::renderToImage() {
    if(fRenderedToImage) return;
    fRenderedToImage = true;
    if(fOpacity < 0.001) return;
    QMatrix scale;
    scale.scale(fResolution, fResolution);
    QMatrix transformRes = fTransform*scale;
    //transformRes.scale(resolution, resolution);
    fGlobalBoundingRect = transformRes.mapRect(fRelBoundingRect);
    foreach(const QRectF &rectT, fOtherGlobalRects) {
        fGlobalBoundingRect = fGlobalBoundingRect.united(rectT);
    }
    fGlobalBoundingRect = fGlobalBoundingRect.
            adjusted(-fEffectsMargin, -fEffectsMargin,
                     fEffectsMargin, fEffectsMargin);
    if(fMaxBoundsEnabled) {
        fGlobalBoundingRect = fGlobalBoundingRect.intersected(
                              scale.mapRect(fMaxBoundsRect));
    }
    QPointF transF = fGlobalBoundingRect.topLeft()/**resolution*/ -
            QPointF(qRound(fGlobalBoundingRect.left()/**resolution*/),
                    qRound(fGlobalBoundingRect.top()/**resolution*/));
    fGlobalBoundingRect.translate(-transF);

    // !!! TEST !!!
    //auto blurProgram = SPtrCreate(BlurProgramCaller)(50., size);
    //fGpuShaders << blurProgram;

//    auto dotsProgram =
//            SPtrCreate(DotProgramCaller)(5., 5., globalBoundingRect.topLeft());
//    fGpuShaders << dotsProgram;
    // !!! TEST !!!

    SkImageInfo info = SkImageInfo::Make(qCeil(fGlobalBoundingRect.width()),
                                         qCeil(fGlobalBoundingRect.height()),
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorTRANSPARENT);
    //sk_sp<SkSurface> rasterSurface(SkSurface::MakeRaster(info));
    SkCanvas rasterCanvas(bitmap);//rasterSurface->getCanvas();
    //rasterCanvas->clear(SK_ColorTRANSPARENT);

    rasterCanvas.translate(qrealToSkScalar(-fGlobalBoundingRect.left()),
                            qrealToSkScalar(-fGlobalBoundingRect.top()));
    rasterCanvas.concat(QMatrixToSkMatrix(transformRes));

    drawSk(&rasterCanvas);
    rasterCanvas.flush();

    fDrawPos = SkPoint::Make(qRound(fGlobalBoundingRect.left()),
                            qRound(fGlobalBoundingRect.top()));

    if(!fPixmapEffects.isEmpty()) {
        foreach(const stdsptr<PixmapEffectRenderData>& effect, fPixmapEffects) {
            effect->applyEffectsSk(bitmap, fResolution);
        }
        clearPixmapEffects();
    }
    bitmap.setImmutable();
    renderedImage = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
}

void BoundingBoxRenderData::_processUpdate() {
    renderToImage();
}

void BoundingBoxRenderData::beforeProcessingStarted() {
    if(!mDataSet) {
        dataSet();
    }

    _ScheduledTask::beforeProcessingStarted();

    BoundingBox *parentBoxT = fParentBox.data();
    if(parentBoxT == nullptr || !fParentIsTarget) return;
    if(nullifyBeforeProcessing())
        parentBoxT->nullifyCurrentRenderData(fRelFrame);
    // qDebug() << "box render started:" << relFrame << parentBoxT->prp_getName();
}

void BoundingBoxRenderData::afterProcessingFinished() {
    if(fMotionBlurTarget != nullptr) {
        fMotionBlurTarget->fOtherGlobalRects << fGlobalBoundingRect;
    }
    BoundingBox *parentBoxT = fParentBox.data();
    if(parentBoxT != nullptr && fParentIsTarget) {
        parentBoxT->renderDataFinished(this);
        // qDebug() << "box render finished:" << relFrame << parentBoxT->prp_getName();
    }
    _ScheduledTask::afterProcessingFinished();
}

void BoundingBoxRenderData::taskQued() {
    BoundingBox *parentBoxT = fParentBox.data();
    if(parentBoxT != nullptr) {
        if(fUseCustomRelFrame) {
            parentBoxT->setupBoundingBoxRenderDataForRelFrameF(
                        fCustomRelFrame, this);
        } else {
            parentBoxT->setupBoundingBoxRenderDataForRelFrameF(
                        fRelFrame, this);
        }
        foreach(const auto& customizer, mRenderDataCustomizerFunctors) {
            (*customizer)(this);
        }
    }
    mDataSet = false;
    if(!mDelayDataSet) {
        dataSet();
    }
    _ScheduledTask::taskQued();
}

void BoundingBoxRenderData::scheduleTaskNow() {
    BoundingBox *parentBoxT = fParentBox.data();
    if(parentBoxT == nullptr) return;
    parentBoxT->scheduleTask(GetAsSPtr(this, _ScheduledTask));
}

void BoundingBoxRenderData::dataSet() {
    if(allDataReady()) {
        mDataSet = true;
        if(!fRelBoundingRectSet) {
            fRelBoundingRectSet = true;
            updateRelBoundingRect();
        }
        BoundingBox *parentBoxT = fParentBox.data();
        if(parentBoxT == nullptr || !fParentIsTarget) return;
        parentBoxT->updateCurrentPreviewDataFromRenderData(this);
    }
}

bool BoundingBoxRenderData::nullifyBeforeProcessing() {
    return fReason != BoundingBox::USER_CHANGE &&
            fReason != BoundingBox::CHILD_USER_CHANGE;
}

RenderDataCustomizerFunctor::RenderDataCustomizerFunctor() {}

RenderDataCustomizerFunctor::~RenderDataCustomizerFunctor() {}

void RenderDataCustomizerFunctor::operator()(BoundingBoxRenderData* data) {
    customize(data);
}

ReplaceTransformDisplacementCustomizer::ReplaceTransformDisplacementCustomizer(
        const qreal &dx, const qreal &dy) {
    mDx = dx;
    mDy = dy;
}

void ReplaceTransformDisplacementCustomizer::customize(
        BoundingBoxRenderData* data) {
    QMatrix transformT = data->fTransform;
    data->fTransform.setMatrix(transformT.m11(), transformT.m12(),
                              transformT.m21(), transformT.m22(),
                              mDx, mDy);
}

MultiplyTransformCustomizer::MultiplyTransformCustomizer(
        const QMatrix &transform, const qreal &opacity) {
    mTransform = transform;
    mOpacity = opacity;
}

void MultiplyTransformCustomizer::customize(BoundingBoxRenderData *data) {
    data->fTransform = mTransform*data->fTransform;
    data->fOpacity *= mOpacity;
}

MultiplyOpacityCustomizer::MultiplyOpacityCustomizer(const qreal &opacity) {
    mOpacity = opacity;
}

void MultiplyOpacityCustomizer::customize(BoundingBoxRenderData *data) {
    data->fOpacity *= mOpacity;
}
