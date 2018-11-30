#include "oileffect.h"
#include "Animators/qrealanimator.h"
#include "fmt_filters.h"

OilEffect::OilEffect(qreal radius) : PixmapEffect("oil", EFFECT_OIL) {
    mRadiusAnimator = SPtrCreate(QrealAnimator)("radius");
    mRadiusAnimator->qra_setValueRange(1., 5.);
    mRadiusAnimator->qra_setCurrentValue(radius);

    ca_addChildAnimator(mRadiusAnimator);
}

PixmapEffectRenderDataSPtr OilEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(OilEffectRenderData)();
    renderData->radius = mRadiusAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}

void OilEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                         const fmt_filters::image &img,
                                         const qreal &scale) {
    Q_UNUSED(imgPtr)
    Q_UNUSED(scale)
    fmt_filters::oil(img, radius);
}
