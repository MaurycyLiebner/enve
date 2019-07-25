#include "colorizeeffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"

stdsptr<PixmapEffectRenderData> ColorizeEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal relFrame, BoxRenderData*) {
    auto renderData = SPtrCreate(ColorizeEffectRenderData)();
    renderData->hue =
            mHueAnimator->getEffectiveValue(relFrame);
    renderData->saturation =
            mSaturationAnimator->getEffectiveValue(relFrame);
    renderData->lightness =
            mLightnessAnimator->getEffectiveValue(relFrame);
    renderData->alpha =
            mAlphaAnimator->getEffectiveValue(relFrame);
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

ColorizeEffect::ColorizeEffect() :
    PixmapEffect("colorize", EFFECT_COLORIZE) {
    mHueAnimator = QrealAnimator::sCreate0to1Animator("hue");

    mSaturationAnimator = QrealAnimator::sCreate0to1Animator("saturation");

    mLightnessAnimator = SPtrCreate(QrealAnimator)(0., -1., 1., 0.01, "lightness");

    mAlphaAnimator = QrealAnimator::sCreate0to1Animator("alpha");

    ca_addChildAnimator(mHueAnimator);
    ca_addChildAnimator(mSaturationAnimator);
    ca_addChildAnimator(mLightnessAnimator);
    ca_addChildAnimator(mAlphaAnimator);
}

void ColorizeEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                              const qreal scale) {
    Q_UNUSED(scale);
    RasterEffects::colorizeHSV(bitmap, hue, saturation, lightness, alpha);
}
