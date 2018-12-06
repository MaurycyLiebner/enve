#include "shadoweffect.h"
#include "fmt_filters.h"
#include "Animators/coloranimator.h"
#include "Properties/boolproperty.h"
#include "Animators/qpointfanimator.h"
#include "pointhelpers.h"

ShadowEffect::ShadowEffect(qreal radius) :
    PixmapEffect("shadow", EFFECT_SHADOW) {
    mHighQuality = SPtrCreate(BoolProperty)("high quality");
    mBlurRadius = SPtrCreate(QrealAnimator)("blur radius");
    mOpacity = SPtrCreate(QrealAnimator)("opacity");
    mColor = SPtrCreate(ColorAnimator)();
    mTranslation = SPtrCreate(QPointFAnimator)("translation");

    mBlurRadius->qra_setCurrentValue(radius);

    mHighQuality->setValue(false);
    ca_addChildAnimator(mHighQuality);

    mBlurRadius->qra_setValueRange(0., 1000.);
    ca_addChildAnimator(mBlurRadius);

    mTranslation->setCurrentPointValue(QPointF(0., 0.));
    mTranslation->setValuesRange(-1000., 1000.);
    ca_addChildAnimator(mTranslation);

    mColor->qra_setCurrentValue(Qt::black);
    ca_addChildAnimator(mColor);

    mOpacity->qra_setValueRange(0., 1000.);
    mOpacity->qra_setCurrentValue(100.);
    ca_addChildAnimator(mOpacity);
//    mScale.setCurrentValue(1.);
//    mScale.setName("scale");
//    mScale.blockPointer();
//    mScale.setValueRange(0., 10.);
//    addChildAnimator(&mScale);
}

stdsptr<PixmapEffectRenderData> ShadowEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(ShadowEffectRenderData)();
    renderData->blurRadius = mBlurRadius->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->hasKeys = mBlurRadius->prp_hasKeys();
    renderData->highQuality = mHighQuality->getValue();
    renderData->color = mColor->getColorAtRelFrameF(relFrame);
    renderData->translation = mTranslation->
            getCurrentEffectivePointValueAtRelFrameF(relFrame);
    renderData->opacity = mOpacity->getCurrentEffectiveValueAtRelFrameF(relFrame)/100.;
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void applyShadow(const SkBitmap &imgPtr,
                 const qreal &scale,
                 const qreal &blurRadius,
                 const QColor &currentColor,
                 const QPointF &trans,
                 const bool &hasKeys,
                 const bool &highQuality,
                 const qreal &opacity = 1.) {
    SkBitmap shadowBitmap;
    shadowBitmap.allocPixels(imgPtr.info());

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kDstIn);
    SkCanvas *shadowCanvas = new SkCanvas(shadowBitmap);
    shadowCanvas->clear(QColorToSkColor(currentColor));
    shadowCanvas->drawBitmap(imgPtr, 0, 0, &paint);
    shadowCanvas->flush();
    delete shadowCanvas;

    SkPixmap shadowPixmap;
    shadowBitmap.peekPixels(&shadowPixmap);
    fmt_filters::image shadowImg(
                static_cast<uint8_t*>(shadowPixmap.writable_addr()),
                shadowBitmap.width(), shadowBitmap.height());

    if(opacity > 1.) {
        fmt_filters::applyBlur(shadowImg, scale,
                  blurRadius, highQuality,
                  hasKeys, opacity);
    } else {
        fmt_filters::applyBlur(shadowImg, scale,
                  blurRadius, highQuality,
                  hasKeys);
        int alphaT = qMin(255, qMax(0, qRound(opacity*255) ));
        paint.setAlpha(static_cast<U8CPU>(alphaT));
    }

    SkCanvas *dstCanvas = new SkCanvas(imgPtr);
    paint.setBlendMode(SkBlendMode::kDstOver);
    dstCanvas->drawBitmap(shadowBitmap,
                          qrealToSkScalar(trans.x()*scale),
                          qrealToSkScalar(trans.y()*scale),
                          &paint);
    dstCanvas->flush();
    delete dstCanvas;
}

void ShadowEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                            const fmt_filters::image &img,
                                            const qreal &scale) {
    Q_UNUSED(img);
    applyShadow(imgPtr, scale,
                blurRadius,
                color,
                translation,
                hasKeys,
                highQuality,
                opacity);
}

qreal ShadowEffect::getMargin() {
    return mBlurRadius->qra_getCurrentValue() +
            pointToLen(mTranslation->getCurrentEffectivePointValue());
}

qreal ShadowEffect::getMarginAtRelFrame(const int &relFrame) {
    return mBlurRadius->qra_getEffectiveValueAtRelFrame(relFrame) +
            pointToLen(mTranslation->getCurrentEffectivePointValueAtRelFrame(relFrame));
}
