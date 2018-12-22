#include "oileffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"

OilEffect::OilEffect() : PixmapEffect("oil", EFFECT_OIL) {
    mRadiusAnimator = SPtrCreate(QrealAnimator)(2., 1., 5., 0.5, "radius");

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
