#include "swirleffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"

SwirlEffect::SwirlEffect(qreal degrees) :
    PixmapEffect("swirl", EFFECT_SWIRL) {
    mDegreesAnimator = SPtrCreate(QrealAnimator)("degrees");

    mDegreesAnimator->setValueRange(-3600., 3600.);
    mDegreesAnimator->setCurrentBaseValue(degrees);
    ca_addChildAnimator(mDegreesAnimator);
}

stdsptr<PixmapEffectRenderData> SwirlEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(SwirlEffectRenderData)();
    renderData->degrees =
            mDegreesAnimator->getEffectiveValue(relFrame);
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void SwirlEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                           const qreal &scale) {
    Q_UNUSED(scale)
    RasterEffects::swirl(bitmap, degrees, RasterEffects::rgba(0, 0, 0, 0));
}
