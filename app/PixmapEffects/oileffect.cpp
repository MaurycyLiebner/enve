#include "oileffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"

OilEffect::OilEffect(qreal radius) : PixmapEffect("oil", EFFECT_OIL) {
    mRadiusAnimator = SPtrCreate(QrealAnimator)("radius");
    mRadiusAnimator->qra_setValueRange(1., 5.);
    mRadiusAnimator->qra_setCurrentValue(radius);

    ca_addChildAnimator(mRadiusAnimator);
}

stdsptr<PixmapEffectRenderData> OilEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(OilEffectRenderData)();
    renderData->radius = mRadiusAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void OilEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                         const qreal &scale) {
    Q_UNUSED(scale)
    RasterEffects::oil(bitmap, radius);
}
