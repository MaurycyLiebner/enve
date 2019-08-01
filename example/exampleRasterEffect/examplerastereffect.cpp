#include "examplerastereffect.h"

qsptr<CustomRasterEffect> createNewestVersionEffect() {
    // Use default, most up to date, version
    return SPtrCreate(ExampleRasterEffect000)();
}

qsptr<CustomRasterEffect> createEffect(
        const CustomIdentifier &identifier) {
    Q_UNUSED(identifier);
    // Choose version based on identifier
    // if(identifier.fVersion == CustomIdentifier::Version{0, 0, 0})
    return SPtrCreate(ExampleRasterEffect000)();
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

#include "enveCore/Animators/qrealanimator.h"
ExampleRasterEffect000::ExampleRasterEffect000() :
    CustomRasterEffect(effectName().toLower()) {
    mRadius = SPtrCreate(QrealAnimator)(10, 0, 999.999, 1, "radius");
    ca_addChild(mRadius);
}

stdsptr<RasterEffectCaller>
ExampleRasterEffect000::getEffectCaller(const qreal relFrame) const {
    const qreal radius = mRadius->getEffectiveValue(relFrame);
    if(isZero4Dec(radius)) return nullptr;
    return SPtrCreate(ExampleRasterEffectCaller000)(radius);
}

QMargins ExampleRasterEffect000::getMarginAtRelFrame(const qreal frame) const {
    const qreal radius = mRadius->getEffectiveValue(frame);
    return QMargins() + qCeil(radius);
}

CustomIdentifier ExampleRasterEffect000::getIdentifier() const {
    return { effectId(), effectName(), { 0, 0, 0 } };
}

using namespace std;
using namespace std::chrono;

void ExampleRasterEffectCaller000::processGpu(QGL33 * const gl,
                                           GpuRenderTools &renderTools,
                                           GpuRenderData &data) {
    Q_UNUSED(gl);
    Q_UNUSED(data);

    const float sigma = mRadius*0.3333333f;
    const auto filter = SkBlurImageFilter::Make(sigma, sigma, nullptr);

    SkPaint paint;
    paint.setImageFilter(filter);

    const auto canvas = renderTools.requestTargetCanvas();
    canvas->clear(SK_ColorTRANSPARENT);
    const auto srcTex = renderTools.requestSrcTextureImageWrapper();
    canvas->drawImage(srcTex, 0, 0, &paint);
    canvas->flush();
}

void ExampleRasterEffectCaller000::processCpu(CpuRenderTools &renderTools,
                                           const CpuRenderData &data) {
    Q_UNUSED(data);

    const float sigma = mRadius*0.3333333f;
    const auto filter = SkBlurImageFilter::Make(sigma, sigma, nullptr);

    SkPaint paint;
    paint.setImageFilter(filter);

    SkBitmap tile;
    renderTools.requestBackupBitmap().extractSubset(&tile, data.fTexTile);
    SkCanvas canvas(tile);
    canvas.clear(SK_ColorTRANSPARENT);

    const int radCeil = static_cast<int>(ceil(mRadius));
    const auto& srcBtmp = renderTools.fSrcDst;
    const auto& texTile = data.fTexTile;
    auto srcRect = texTile.makeOutset(radCeil, radCeil);
    if(srcRect.intersect(srcRect, srcBtmp.bounds())) {
        SkBitmap tileSrc;
        srcBtmp.extractSubset(&tileSrc, srcRect);
        canvas.drawBitmap(tileSrc,
                          srcRect.left() - texTile.left(),
                          srcRect.top() - texTile.top(), &paint);
    }

    renderTools.swap();
}
