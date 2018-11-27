#include "contrasteffect.h"
#include "fmt_filters.h"
#include "Animators/qrealanimator.h"

ContrastEffect::ContrastEffect(qreal contrast) :
    PixmapEffect("contrast", EFFECT_CONTRAST) {
    mContrastAnimator = SPtrCreate(QrealAnimator)("contrast");

    mContrastAnimator->qra_setValueRange(-255., 255.);
    mContrastAnimator->qra_setCurrentValue(contrast);
    ca_addChildAnimator(mContrastAnimator);
}

PixmapEffectRenderDataSPtr ContrastEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(ContrastEffectRenderData)();
    renderData->contrast =
            mContrastAnimator->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->hasKeys = mContrastAnimator->prp_hasKeys();
    return renderData;
}

void ContrastEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                              const fmt_filters::image &img,
                                              const qreal &scale) {
    Q_UNUSED(imgPtr);
    Q_UNUSED(scale);
    if(hasKeys) {
        fmt_filters::anim_contrast(img, contrast);
    } else {
        fmt_filters::contrast(img, qRound(contrast));
    }
}
