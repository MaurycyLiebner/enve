#include "brightnesseffect.h"
#include "rastereffects.h"
#include "Animators/qrealanimator.h"

BrightnessEffect::BrightnessEffect(qreal brightness) :
    PixmapEffect("brightness", EFFECT_BRIGHTNESS) {
    mBrightnessAnimator = SPtrCreate(QrealAnimator)("brightness");

    mBrightnessAnimator->qra_setValueRange(-255., 255.);
    mBrightnessAnimator->qra_setCurrentValue(brightness);
    ca_addChildAnimator(mBrightnessAnimator);
}

stdsptr<PixmapEffectRenderData> BrightnessEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData =
            SPtrCreate(BrightnessEffectRenderData)();
    renderData->brightness =
            mBrightnessAnimator->getCurrentEffectiveValueAtRelFrame(relFrame);
    renderData->hasKeys = mBrightnessAnimator->anim_hasKeys();
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void BrightnessEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                                const qreal &scale) {
    Q_UNUSED(scale);
    if(hasKeys) {
        RasterEffects::anim_brightness(bitmap, brightness);
    } else {
        RasterEffects::brightness(bitmap, qRound(brightness));
    }
}
