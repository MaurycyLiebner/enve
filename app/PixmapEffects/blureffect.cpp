#include "blureffect.h"
#include "Animators/qrealanimator.h"
#include "Properties/boolproperty.h"
#include "rastereffects.h"

BlurEffect::BlurEffect() : PixmapEffect("blur", EFFECT_BLUR) {
    mBlurRadius = SPtrCreate(QrealAnimator)(0., 999., 1., "radius");
    mBlurRadius->qra_setCurrentValue(10.);

    mHighQuality = SPtrCreate(BoolProperty)("high quality");
    mHighQuality->setValue(false);

    ca_addChildAnimator(mHighQuality);
    ca_addChildAnimator(mBlurRadius);
}

void BlurEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                          const qreal &scale) {
    RasterEffects::applyBlur(bitmap, scale, blurRadius, highQuality, hasKeys);
}

qreal BlurEffect::getMargin() {
    return mBlurRadius->qra_getCurrentValue();
}

qreal BlurEffect::getMarginAtRelFrame(const int &relFrame) {
    return mBlurRadius->qra_getEffectiveValueAtRelFrame(relFrame);
}

stdsptr<PixmapEffectRenderData> BlurEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(BlurEffectRenderData)();
    renderData->blurRadius = mBlurRadius->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->hasKeys = mBlurRadius->prp_hasKeys();
    renderData->highQuality = mHighQuality->getValue();
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}
