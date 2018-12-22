#include "colorizeeffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"

stdsptr<PixmapEffectRenderData> ColorizeEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(ColorizeEffectRenderData)();
    renderData->hue =
            mHueAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->saturation =
            mSaturationAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->lightness =
            mLightnessAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->alpha =
            mAlphaAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

ColorizeEffect::ColorizeEffect() :
    PixmapEffect("colorize", EFFECT_COLORIZE) {
    mHueAnimator = QrealAnimator::create0to1Animator("hue");

    mSaturationAnimator = QrealAnimator::create0to1Animator("saturation");

    mLightnessAnimator = SPtrCreate(QrealAnimator)(0., -1., 1., 0.01, "lightness");

    mAlphaAnimator = QrealAnimator::create0to1Animator("alpha");

    ca_addChildAnimator(mHueAnimator);
    ca_addChildAnimator(mSaturationAnimator);
    ca_addChildAnimator(mLightnessAnimator);
    ca_addChildAnimator(mAlphaAnimator);
}

void ColorizeEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                              const qreal &scale) {
    Q_UNUSED(scale);
    RasterEffects::colorizeHSV(bitmap, hue, saturation, lightness, alpha);
}
