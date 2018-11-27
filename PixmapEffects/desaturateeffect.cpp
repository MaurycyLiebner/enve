#include "desaturateeffect.h"
#include "Animators/qrealanimator.h"
#include "fmt_filters.h"

DesaturateEffect::DesaturateEffect(qreal radius) :
    PixmapEffect("desaturate", EFFECT_DESATURATE) {
    mInfluenceAnimator = SPtrCreate(QrealAnimator)("factor");
    mInfluenceAnimator->qra_setValueRange(0., 1.);
    mInfluenceAnimator->qra_setCurrentValue(radius);

    ca_addChildAnimator(mInfluenceAnimator);
}

PixmapEffectRenderDataSPtr DesaturateEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(DesaturateEffectRenderData)();
    renderData->influence =
            mInfluenceAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}

void DesaturateEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                                const fmt_filters::image &img,
                                                const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    fmt_filters::desaturate(img, influence);
}
