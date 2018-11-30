#include "blureffect.h"
#include "Animators/qrealanimator.h"
#include "Properties/boolproperty.h"
#include "fmt_filters.h"

BlurEffect::BlurEffect() : PixmapEffect("blur", EFFECT_BLUR) {
    mBlurRadius = SPtrCreate(QrealAnimator)("radius");
    mBlurRadius->qra_setCurrentValue(10.);
    mBlurRadius->qra_setValueRange(0., 1000.);

    mHighQuality = SPtrCreate(BoolProperty)("high quality");
    mHighQuality->setValue(false);

    ca_addChildAnimator(mHighQuality);
    ca_addChildAnimator(mBlurRadius);
}

void BlurEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                          const fmt_filters::image &img,
                                          const qreal &scale) {
    Q_UNUSED(imgPtr)
    fmt_filters::applyBlur(img, scale, blurRadius, highQuality, hasKeys);
}

qreal BlurEffect::getMargin() {
    return mBlurRadius->qra_getCurrentValue();
}

qreal BlurEffect::getMarginAtRelFrame(const int &relFrame) {
    return mBlurRadius->qra_getEffectiveValueAtRelFrame(relFrame);
}

PixmapEffectRenderDataSPtr BlurEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(BlurEffectRenderData)();
    renderData->blurRadius = mBlurRadius->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->hasKeys = mBlurRadius->prp_hasKeys();
    renderData->highQuality = mHighQuality->getValue();
    return renderData;
}
