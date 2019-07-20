#include "desaturateeffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"

DesaturateEffect::DesaturateEffect() :
    PixmapEffect("desaturate", EFFECT_DESATURATE) {
    mInfluenceAnimator = QrealAnimator::sCreate0to1Animator("factor");
    mInfluenceAnimator->setCurrentBaseValue(0.2);

    ca_addChildAnimator(mInfluenceAnimator);

    setPropertyForGUI(mInfluenceAnimator.get());
}

stdsptr<PixmapEffectRenderData> DesaturateEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(DesaturateEffectRenderData)();
    renderData->influence =
            mInfluenceAnimator->getEffectiveValue(relFrame);
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void DesaturateEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                                const qreal scale) {
    Q_UNUSED(scale);
    RasterEffects::desaturate(bitmap, influence);
}
