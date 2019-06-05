#include "blureffect.h"
#include "skia/skiahelpers.h"

#include "Animators/qrealanimator.h"
#include "Properties/boolproperty.h"

BlurEffect::BlurEffect() : PixmapEffect("blur", EFFECT_BLUR) {
    mBlurRadius = SPtrCreate(QrealAnimator)(10, 0, 300, 1, "radius");

    ca_addChildAnimator(mBlurRadius);

    setPropertyForGUI(mBlurRadius.get());
}

void BlurEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                          const qreal scale) {
    const SkScalar sigma = toSkScalar(fBlurRadius*0.3333*scale);
    const auto src = SkiaHelpers::makeCopy(bitmap);
    SkCanvas canvas(bitmap);
    canvas.clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    const auto filter = SkBlurImageFilter::Make(sigma, sigma, nullptr);
    paint.setImageFilter(filter);
    canvas.drawBitmap(src, 0, 0, &paint);
}

qreal BlurEffect::getMargin() {
    return mBlurRadius->getCurrentBaseValue();
}

qreal BlurEffect::getMarginAtRelFrame(const int relFrame) {
    return mBlurRadius->getEffectiveValue(relFrame);
}

stdsptr<PixmapEffectRenderData> BlurEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(BlurEffectRenderData)();
    renderData->fBlurRadius = mBlurRadius->getEffectiveValue(relFrame);
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}
