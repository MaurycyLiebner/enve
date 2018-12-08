#include "boundingboxrenderdata.h"
#include "boundingbox.h"
#include "PixmapEffects/rastereffects.h"

BoundingBoxRenderData::BoundingBoxRenderData(BoundingBox *parentBoxT) {
    if(parentBoxT == nullptr) return;
    parentBox = parentBoxT;
}

BoundingBoxRenderData::~BoundingBoxRenderData() {}

#include "skimagecopy.h"
void BoundingBoxRenderData::copyFrom(BoundingBoxRenderData *src) {
    globalBoundingRect = src->globalBoundingRect;
    transform = src->transform;
    parentTransform = src->parentTransform;
    customRelFrame = src->customRelFrame;
    useCustomRelFrame = src->useCustomRelFrame;
    relFrame = src->relFrame;
    relBoundingRect = src->relBoundingRect;
    relTransform = src->relTransform;
    renderedToImage = src->renderedToImage;
    blendMode = src->blendMode;
    drawPos = src->drawPos;
    opacity = src->opacity;
    resolution = src->resolution;
    renderedImage = makeSkImageCopy(src->renderedImage);
    mFinished = true;
    relBoundingRectSet = true;
    copied = true;
}

stdsptr<BoundingBoxRenderData> BoundingBoxRenderData::makeCopy() {
    BoundingBox *parentBoxT = parentBox.data();
    if(parentBoxT == nullptr) return nullptr;
    stdsptr<BoundingBoxRenderData> copy = parentBoxT->createRenderData();
    copy->copyFrom(this);
    return copy;
}

void BoundingBoxRenderData::updateRelBoundingRect() {
    BoundingBox *parentBoxT = parentBox.data();
    if(parentBoxT == nullptr) return;
    relBoundingRect = parentBoxT->getRelBoundingRectAtRelFrame(relFrame);
}

void BoundingBoxRenderData::drawRenderedImageForParent(SkCanvas *canvas) {
    if(opacity < 0.001) return;
    canvas->save();
    SkScalar invScale = 1.f/qrealToSkScalar(resolution);
    canvas->scale(invScale, invScale);
    renderToImage();
    SkPaint paint;
    paint.setAlpha(static_cast<U8CPU>(qRound(opacity*2.55)));
    paint.setBlendMode(blendMode);
    //paint.setAntiAlias(true);
    //paint.setFilterQuality(kHigh_SkFilterQuality);
    if(blendMode == SkBlendMode::kDstIn ||
       blendMode == SkBlendMode::kSrcIn ||
       blendMode == SkBlendMode::kDstATop) {
        SkPaint paintT;
        paintT.setBlendMode(blendMode);
        paintT.setColor(SK_ColorTRANSPARENT);
        SkPath path;
        path.addRect(SkRect::MakeXYWH(drawPos.x(), drawPos.y(),
                                      renderedImage->width(),
                                      renderedImage->height()));
        path.toggleInverseFillType();
        canvas->drawPath(path, paintT);
    }
    canvas->drawImage(renderedImage,
                      drawPos.x(), drawPos.y(),
                      &paint);
    canvas->restore();
}

void BoundingBoxRenderData::renderToImage() {
    if(renderedToImage) return;
    renderedToImage = true;
    if(opacity < 0.001) return;
    QMatrix scale;
    scale.scale(resolution, resolution);
    QMatrix transformRes = transform*scale;
    //transformRes.scale(resolution, resolution);
    globalBoundingRect = transformRes.mapRect(relBoundingRect);
    foreach(const QRectF &rectT, otherGlobalRects) {
        globalBoundingRect = globalBoundingRect.united(rectT);
    }
    globalBoundingRect = globalBoundingRect.
            adjusted(-effectsMargin, -effectsMargin,
                     effectsMargin, effectsMargin);
    if(maxBoundsEnabled) {
        globalBoundingRect = globalBoundingRect.intersected(
                              scale.mapRect(maxBoundsRect));
    }
    QPointF transF = globalBoundingRect.topLeft()/**resolution*/ -
            QPointF(qRound(globalBoundingRect.left()/**resolution*/),
                    qRound(globalBoundingRect.top()/**resolution*/));
    globalBoundingRect.translate(-transF);
    SkImageInfo info = SkImageInfo::Make(qCeil(globalBoundingRect.width()),
                                         qCeil(globalBoundingRect.height()),
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorTRANSPARENT);
    //sk_sp<SkSurface> rasterSurface(SkSurface::MakeRaster(info));
    SkCanvas rasterCanvas(bitmap);//rasterSurface->getCanvas();
    //rasterCanvas->clear(SK_ColorTRANSPARENT);

    rasterCanvas.translate(qrealToSkScalar(-globalBoundingRect.left()),
                            qrealToSkScalar(-globalBoundingRect.top()));
    rasterCanvas.concat(QMatrixToSkMatrix(transformRes));

    drawSk(&rasterCanvas);
    rasterCanvas.flush();

    drawPos = SkPoint::Make(qRound(globalBoundingRect.left()),
                            qRound(globalBoundingRect.top()));

    if(!pixmapEffects.isEmpty()) {
        foreach(const stdsptr<PixmapEffectRenderData>& effect, pixmapEffects) {
            effect->applyEffectsSk(bitmap, resolution);
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

void BoundingBoxRenderData::beforeUpdate() {
    if(!mDataSet) {
        dataSet();
    }
    _ScheduledExecutor::beforeUpdate();

    BoundingBox *parentBoxT = parentBox.data();
    if(parentBoxT == nullptr || !parentIsTarget) return;
    if(reason != BoundingBox::USER_CHANGE &&
            reason != BoundingBox::CHILD_USER_CHANGE)
        parentBoxT->nullifyCurrentRenderData(relFrame);
    // qDebug() << "box render started:" << relFrame << parentBoxT->prp_getName();
}

void BoundingBoxRenderData::afterUpdate() {
    if(motionBlurTarget != nullptr) {
        motionBlurTarget->otherGlobalRects << globalBoundingRect;
    }
    BoundingBox *parentBoxT = parentBox.data();
    if(parentBoxT != nullptr && parentIsTarget) {
        parentBoxT->renderDataFinished(this);
        // qDebug() << "box render finished:" << relFrame << parentBoxT->prp_getName();
    }
    _ScheduledExecutor::afterUpdate();
}

void BoundingBoxRenderData::parentBeingProcessed() {
    mFinished = false;
    mSchedulerAdded = true;
    schedulerProccessed();
}

void BoundingBoxRenderData::schedulerProccessed() {
    BoundingBox *parentBoxT = parentBox.data();
    if(parentBoxT != nullptr) {
        if(useCustomRelFrame) {
            parentBoxT->setupBoundingBoxRenderDataForRelFrameF(
                        customRelFrame, this);
        } else {
            parentBoxT->setupBoundingBoxRenderDataForRelFrameF(
                        relFrame, this);
        }
        foreach(const auto& customizer, mRenderDataCustomizerFunctors) {
            (*customizer)(this);
        }
    }
    mDataSet = false;
    if(!mDelayDataSet) {
        dataSet();
    }
    _ScheduledExecutor::schedulerProccessed();
}

void BoundingBoxRenderData::addSchedulerNow() {
    BoundingBox *parentBoxT = parentBox.data();
    if(parentBoxT == nullptr) return;
    parentBoxT->addScheduler(GetAsSPtr(this, _ScheduledExecutor));
}

void BoundingBoxRenderData::dataSet() {
    if(allDataReady()) {
        mDataSet = true;
        if(!relBoundingRectSet) {
            relBoundingRectSet = true;
            updateRelBoundingRect();
        }
        BoundingBox *parentBoxT = parentBox.data();
        if(parentBoxT == nullptr || !parentIsTarget) return;
        parentBoxT->updateCurrentPreviewDataFromRenderData(this);
    }
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
    QMatrix transformT = data->transform;
    data->transform.setMatrix(transformT.m11(), transformT.m12(),
                              transformT.m21(), transformT.m22(),
                              mDx, mDy);
}

MultiplyTransformCustomizer::MultiplyTransformCustomizer(
        const QMatrix &transform, const qreal &opacity) {
    mTransform = transform;
    mOpacity = opacity;
}

void MultiplyTransformCustomizer::customize(BoundingBoxRenderData *data) {
    data->transform = mTransform*data->transform;
    data->opacity *= mOpacity;
}

MultiplyOpacityCustomizer::MultiplyOpacityCustomizer(const qreal &opacity) {
    mOpacity = opacity;
}

void MultiplyOpacityCustomizer::customize(BoundingBoxRenderData *data) {
    data->opacity *= mOpacity;
}
