#include "colorizeeffect.h"
#include "Animators/qrealanimator.h"
#include "fmt_filters.h"

PixmapEffectRenderDataSPtr ColorizeEffect::getPixmapEffectRenderDataForRelFrameF(
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
    return renderData;
}

ColorizeEffect::ColorizeEffect() :
    PixmapEffect("colorize", EFFECT_COLORIZE) {
    mHueAnimator = SPtrCreate(QrealAnimator)("hue");
    mHueAnimator->qra_setValueRange(0., 1.);
    mHueAnimator->setPrefferedValueStep(0.01);

    mSaturationAnimator = SPtrCreate(QrealAnimator)("saturation");
    mSaturationAnimator->qra_setValueRange(0., 1.);
    mSaturationAnimator->setPrefferedValueStep(0.01);

    mLightnessAnimator = SPtrCreate(QrealAnimator)("lightness");
    mLightnessAnimator->qra_setValueRange(-1., 1.);
    mLightnessAnimator->setPrefferedValueStep(0.01);

    mAlphaAnimator = SPtrCreate(QrealAnimator)("alpha");
    mAlphaAnimator->qra_setValueRange(0., 1.);
    mAlphaAnimator->setPrefferedValueStep(0.01);

    ca_addChildAnimator(mHueAnimator);
    ca_addChildAnimator(mSaturationAnimator);
    ca_addChildAnimator(mLightnessAnimator);
    ca_addChildAnimator(mAlphaAnimator);
}

void ColorizeEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                              const fmt_filters::image &img,
                                              const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::colorizeHSV(img,
                             hue, saturation, lightness, alpha);
}
