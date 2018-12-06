#include "implodeeffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"

ImplodeEffect::ImplodeEffect(qreal radius) :
    PixmapEffect("implode", EFFECT_IMPLODE) {
    mFactorAnimator = SPtrCreate(QrealAnimator)("factor");
    mFactorAnimator->qra_setValueRange(0., 100.);
    mFactorAnimator->qra_setCurrentValue(radius);

    ca_addChildAnimator(mFactorAnimator);
}

stdsptr<PixmapEffectRenderData> ImplodeEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(ImplodeEffectRenderData)();
    renderData->factor =
            mFactorAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void ImplodeEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                             const qreal &scale) {
    Q_UNUSED(scale);
    RasterEffects::implode(bitmap, factor, RasterEffects::rgba(0, 0, 0, 0));
}
