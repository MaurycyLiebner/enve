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

    mOpacity->setValueRange(100, 1000);
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
        const qreal relFrame, BoxRenderData*) {
    auto renderData = SPtrCreate(ShadowEffectRenderData)();
    renderData->fBlurRadius = mBlurRadius->getEffectiveValue(relFrame);
    renderData->fColor = mColor->getColor(relFrame);
    renderData->fTranslation = mTranslation->
            getEffectiveValue(relFrame);
    renderData->fOpacity = mOpacity->getEffectiveValue(relFrame)/100.;
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void ShadowEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                            const qreal scale) {
    const float sigma = toSkScalar(fBlurRadius*0.3333*scale);
    const auto src = SkiaHelpers::makeCopy(bitmap);
    SkCanvas canvas(bitmap);
    canvas.clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    const auto filter = SkDropShadowImageFilter::Make(
                toSkScalar(fTranslation.x()), toSkScalar(fTranslation.y()),
                sigma, sigma, toSkColor(fColor),
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
                nullptr);
    paint.setImageFilter(filter);
    const float opacityM[20] = {
        1, 0, 0, 0, 0,
        0, 1, 0, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 0, toSkScalar(fOpacity), 0};
    paint.setColorFilter(SkColorFilters::Matrix(opacityM));
    canvas.drawBitmap(src, 0, 0, &paint);
}

QMarginsF ShadowEffect::getMarginAtRelFrame(const qreal relFrame) {
    const qreal blur = mBlurRadius->getEffectiveValue(relFrame);
    const QPointF trans = mTranslation->getEffectiveValue(relFrame);

    const qreal iL = qMax(0., blur - trans.x());
    const qreal iT = qMax(0., blur - trans.y());
    const qreal iR = qMax(0., blur + trans.x());
    const qreal iB = qMax(0., blur + trans.y());

    return QMarginsF(iL, iT, iR, iB);;
}
