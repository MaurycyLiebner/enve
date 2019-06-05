#include "contrasteffect.h"
#include "rastereffects.h"
#include "Animators/qrealanimator.h"

ContrastEffect::ContrastEffect(qreal contrast) :
    PixmapEffect("contrast", EFFECT_CONTRAST) {
    mContrastAnimator = SPtrCreate(QrealAnimator)("contrast");

    mContrastAnimator->setValueRange(-255, 255);
    mContrastAnimator->setCurrentBaseValue(contrast);
    ca_addChildAnimator(mContrastAnimator);

    setPropertyForGUI(mContrastAnimator.get());
}

stdsptr<PixmapEffectRenderData> ContrastEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(ContrastEffectRenderData)();
    renderData->contrast =
            mContrastAnimator->getEffectiveValue(relFrame);
    renderData->hasKeys = mContrastAnimator->anim_hasKeys();
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void ContrastEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                              const qreal scale) {
    Q_UNUSED(scale);
    if(hasKeys) {
        RasterEffects::anim_contrast(bitmap, contrast);
    } else {
        RasterEffects::contrast(bitmap, qRound(contrast));
    }
}
