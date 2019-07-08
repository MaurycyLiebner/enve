#include "examplegpueffect.h"

qsptr<CustomGpuEffect> createNewestVersionEffect() {
    // Use default, most up to date, version
    return SPtrCreate(ExampleGpuEffect000)();
}

qsptr<CustomGpuEffect> createEffect(
        const CustomIdentifier &identifier) {
    Q_UNUSED(identifier);
    // Choose version based on identifier
    // if(identifier.fVersion == CustomIdentifier::Version{0, 0, 0})
    return SPtrCreate(ExampleGpuEffect000)();
}

// Returned value must be unique, lets enve distinguish effects
QString effectId() {
    return "iz4784jsvg6fw7bc3clb";
}

// Name of your effect used in UI
QString effectName() {
    return "Example";
}

// here specify your effect's most up to date version
CustomIdentifier::Version effectVersion() {
    return { 0, 0, 0 };
}

CustomIdentifier effectIdentifier() {
    return { effectId(), effectName(), effectVersion() };
}

bool supports(const CustomIdentifier &identifier) {
    if(identifier.fEffectId != effectId()) return false;
    if(identifier.fEffectName != effectName()) return false;
    return identifier.fVersion == effectVersion();
}

#include "Animators/qrealanimator.h"
ExampleGpuEffect000::ExampleGpuEffect000() :
    CustomGpuEffect(effectName().toLower()) {
    mRadius = SPtrCreate(QrealAnimator)(10, 0, 999.999, 1, "radius");
    ca_addChildAnimator(mRadius);
}

stdsptr<GPURasterEffectCaller>
        ExampleGpuEffect000::getEffectCaller(const qreal relFrame) const {
    const SkScalar radius = toSkScalar(mRadius->getEffectiveValue(relFrame));
    return SPtrCreate(ExampleGpuEffectCaller000)(radius);
}

QMarginsF ExampleGpuEffect000::getMarginAtRelFrame(const qreal frame) {
    const qreal radius = mRadius->getEffectiveValue(frame);
    return QMarginsF(radius, radius, radius, radius);
}

CustomIdentifier ExampleGpuEffect000::getIdentifier() const {
    return { effectId(), effectName(), { 0, 0, 0 } };
}

void ExampleGpuEffectCaller000::render(QGL33c * const gl,
                                       GpuRenderTools &renderTools,
                                       GpuRenderData &data) {
    Q_UNUSED(gl);
    Q_UNUSED(data);
    const auto canvas = renderTools.requestTargetCanvas();
    const auto srcTex = renderTools.requestSrcTextureImageWrapper();

    canvas->clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    const SkScalar sigma = mRadius*0.3333333f;
    const auto filter = SkBlurImageFilter::Make(sigma, sigma, nullptr);
    paint.setImageFilter(filter);
    const uint fRadius = static_cast<uint>(mRadius);
    const SkRect rect = SkRect::MakeXYWH(fRadius, fRadius,
                                         data.fWidth - fRadius,
                                         data.fHeight - fRadius);
    canvas->drawImageRect(srcTex, rect, rect, &paint,
                          SkCanvas::kFast_SrcRectConstraint);
    canvas->flush();
}
