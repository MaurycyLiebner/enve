#include "shadoweffect.h"
#include "skia/skiahelpers.h"
#include "Animators/coloranimator.h"
#include "Properties/boolproperty.h"
#include "Animators/qpointfanimator.h"
#include "pointhelpers.h"

ShadowEffect::ShadowEffect() :
    PixmapEffect("shadow", EFFECT_SHADOW) {
    mBlurRadius = SPtrCreate(QrealAnimator)("blur radius");
    mOpacity = SPtrCreate(QrealAnimator)("opacity");
    mColor = SPtrCreate(ColorAnimator)();
    mTranslation = SPtrCreate(QPointFAnimator)("translation");

    mBlurRadius->setCurrentBaseValue(10);

    mBlurRadius->setValueRange(0, 300);
    ca_addChildAnimator(mBlurRadius);

    mTranslation->setBaseValue(QPointF(0, 0));
    mTranslation->setValuesRange(-1000, 1000);
    ca_addChildAnimator(mTranslation);

    mColor->qra_setCurrentValue(Qt::black);
    ca_addChildAnimator(mColor);

    mOpacity->setValueRange(0, 1000);
    mOpacity->setCurrentBaseValue(100);
    ca_addChildAnimator(mOpacity);

    setPropertyForGUI(mColor.get());
//    mScale.setCurrentValue(1.);
//    mScale.setName("scale");
//    mScale.blockPointer();
//    mScale.setValueRange(0., 10.);
//    addChildAnimator(&mScale);
}

stdsptr<PixmapEffectRenderData> ShadowEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(ShadowEffectRenderData)();
    renderData->blurRadius = mBlurRadius->getEffectiveValue(relFrame);
    renderData->color = mColor->getColorAtRelFrame(relFrame);
    renderData->translation = mTranslation->
            getEffectiveValueAtRelFrame(relFrame);
    renderData->opacity = mOpacity->getEffectiveValue(relFrame)/100.;
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void ShadowEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                            const qreal &scale) {
    const SkScalar sigma = toSkScalar(blurRadius*0.3333*scale);
    const auto src = SkiaHelpers::makeCopy(bitmap);
    SkCanvas canvas(bitmap);
    canvas.clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    const auto filter = SkDropShadowImageFilter::Make(
                toSkScalar(translation.x()), toSkScalar(translation.y()),
                sigma, sigma, toSkColor(color),
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
                nullptr);
    paint.setImageFilter(filter);
    canvas.drawBitmap(src, 0, 0, &paint);
}

qreal ShadowEffect::getMargin() {
    return mBlurRadius->getCurrentBaseValue() +
            pointToLen(mTranslation->getEffectiveValue());
}

qreal ShadowEffect::getMarginAtRelFrame(const int &relFrame) {
    return mBlurRadius->getEffectiveValue(relFrame) +
            pointToLen(mTranslation->getEffectiveValueAtRelFrame(relFrame));
}
