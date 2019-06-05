#include "implodeeffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"

ImplodeEffect::ImplodeEffect() :
    PixmapEffect("implode", EFFECT_IMPLODE) {
    mFactorAnimator = SPtrCreate(QrealAnimator)(10., 0., 100., 1., "factor");

    ca_addChildAnimator(mFactorAnimator);
}

stdsptr<PixmapEffectRenderData> ImplodeEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(ImplodeEffectRenderData)();
    renderData->factor =
            mFactorAnimator->getEffectiveValue(relFrame);
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void ImplodeEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                             const qreal scale) {
    Q_UNUSED(scale);
    RasterEffects::implode(bitmap, factor, RasterEffects::rgba(0, 0, 0, 0));
}
