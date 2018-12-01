#include "brightnesseffect.h"
#include "fmt_filters.h"
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
            mBrightnessAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->hasKeys = mBrightnessAnimator->prp_hasKeys();
    return renderData;
}

void BrightnessEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                                const fmt_filters::image &img,
                                                const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    if(hasKeys) {
        fmt_filters::anim_brightness(img, brightness);
    } else {
        fmt_filters::brightness(img, qRound(brightness));
    }
}
