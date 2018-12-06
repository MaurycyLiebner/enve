#include "desaturateeffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"

DesaturateEffect::DesaturateEffect(qreal radius) :
    PixmapEffect("desaturate", EFFECT_DESATURATE) {
    mInfluenceAnimator = SPtrCreate(QrealAnimator)("factor");
    mInfluenceAnimator->qra_setValueRange(0., 1.);
    mInfluenceAnimator->qra_setCurrentValue(radius);

    ca_addChildAnimator(mInfluenceAnimator);
}

stdsptr<PixmapEffectRenderData> DesaturateEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(DesaturateEffectRenderData)();
    renderData->influence =
            mInfluenceAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void DesaturateEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                                const qreal &scale) {
    Q_UNUSED(scale);
    RasterEffects::desaturate(bitmap, influence);
}
