#include "implodeeffect.h"
#include "Animators/qrealanimator.h"
#include "fmt_filters.h"

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
    renderData->factor = mFactorAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}

void ImplodeEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                             const fmt_filters::image &img,
                                             const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::implode(img, factor, fmt_filters::rgba(0, 0, 0, 0));
}
