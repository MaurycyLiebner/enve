#include "swirleffect.h"
#include "Animators/qrealanimator.h"
#include "fmt_filters.h"

SwirlEffect::SwirlEffect(qreal degrees) :
    PixmapEffect("swirl", EFFECT_SWIRL) {
    mDegreesAnimator = SPtrCreate(QrealAnimator)("degrees");

    mDegreesAnimator->qra_setValueRange(-3600., 3600.);
    mDegreesAnimator->qra_setCurrentValue(degrees);
    ca_addChildAnimator(mDegreesAnimator);
}

stdsptr<PixmapEffectRenderData> SwirlEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(SwirlEffectRenderData)();
    renderData->degrees = mDegreesAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    return renderData;
}

void SwirlEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                           const fmt_filters::image &img,
                                           const qreal &scale) {
    Q_UNUSED(imgPtr)
    Q_UNUSED(scale)
    fmt_filters::swirl(img, degrees, fmt_filters::rgba(0, 0, 0, 0));
}
