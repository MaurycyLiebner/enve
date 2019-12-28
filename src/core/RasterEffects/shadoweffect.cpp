#include "shadoweffect.h"


class ShadowEffectCaller : public RasterEffectCaller {
public:
    ShadowEffectCaller(const HardwareSupport hwSupport,
                       const qreal radius,
                       const QColor& color,
                       const QPointF& translation,
                       const qreal opacity,
                       const QMargins& margin) :
        RasterEffectCaller(hwSupport, true, margin),
        mRadius(static_cast<float>(radius)),
        mColor(toSkColor(color)),
        mTranslation(toSkPoint(translation)),
        mOpacity(static_cast<float>(opacity)) {}

    void processGpu(QGL33 * const gl,
                    GpuRenderTools& renderTools,
                    GpuRenderData& data);
    void processCpu(CpuRenderTools& renderTools,
                    const CpuRenderData &data);
private:
    void setupPaint(SkPaint& paint) const;

    const float mRadius;
    const SkColor mColor;
    const SkPoint mTranslation;
    const SkScalar mOpacity;
};

ShadowEffect::ShadowEffect() :
    RasterEffect("shadow", HardwareSupport::gpuPreffered,
                 false, RasterEffectType::SHADOW) {
    mBlurRadius = enve::make_shared<QrealAnimator>("blur radius");
    mOpacity = enve::make_shared<QrealAnimator>("opacity");
    mColor = enve::make_shared<ColorAnimator>();
    mTranslation = enve::make_shared<QPointFAnimator>("translation");

    mBlurRadius->setValueRange(0, 300);
    mBlurRadius->setCurrentBaseValue(10);
    ca_addChild(mBlurRadius);

    mTranslation->setValuesRange(-1000, 1000);
    mTranslation->setBaseValue(QPointF(0, 0));
    ca_addChild(mTranslation);

    mColor->qra_setCurrentValue(Qt::black);
    ca_addChild(mColor);

    mOpacity->setValueRange(0, 9);
    mOpacity->setCurrentBaseValue(1);
    ca_addChild(mOpacity);

    ca_setGUIProperty(mColor.data());
}


stdsptr<RasterEffectCaller>
ShadowEffect::getEffectCaller(const qreal relFrame, const qreal resolution,
                              const qreal influence) const {
    const qreal blur = mBlurRadius->getEffectiveValue(relFrame)*resolution;
    const QColor color = mColor->getColor(relFrame);
    const QPointF trans = mTranslation->getEffectiveValue(relFrame)*resolution;
    const qreal opacity = mOpacity->getEffectiveValue(relFrame)*influence;

    const int iL = qMax(0, qCeil(blur - trans.x()));
    const int iT = qMax(0, qCeil(blur - trans.y()));
    const int iR = qMax(0, qCeil(blur + trans.x()));
    const int iB = qMax(0, qCeil(blur + trans.y()));

    return enve::make_shared<ShadowEffectCaller>(
                instanceHwSupport(), blur,
                color, trans, opacity,
                QMargins(iL, iT, iR, iB));
}

void ShadowEffectCaller::setupPaint(SkPaint &paint) const {
    const float sigma = mRadius*0.3333333f;
    const auto filter = SkDropShadowImageFilter::Make(
                mTranslation.x(), mTranslation.y(),
                sigma, sigma, toSkColor(mColor),
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
                nullptr);
    paint.setImageFilter(filter);
    const float opacityM[20] = {
        1, 0, 0, 0, 0,
        0, 1, 0, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 0, mOpacity, 0};
    paint.setColorFilter(SkColorFilters::Matrix(opacityM));
}

void ShadowEffectCaller::processGpu(QGL33 * const gl,
                                    GpuRenderTools &renderTools,
                                    GpuRenderData &data) {
    Q_UNUSED(gl)
    Q_UNUSED(data)

    renderTools.switchToSkia();
    const auto canvas = renderTools.requestTargetCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    const auto srcTex = renderTools.requestSrcTextureImageWrapper();
    canvas->drawImage(srcTex, 0, 0);

    SkPaint paint;
    setupPaint(paint);
    canvas->drawImage(srcTex, 0, 0, &paint);
    canvas->flush();

    renderTools.swapTextures();
}

void ShadowEffectCaller::processCpu(CpuRenderTools &renderTools,
                                    const CpuRenderData &data) {
    Q_UNUSED(data)
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
        const int drawX = srcRect.left() - texTile.left();
        const int drawY = srcRect.top() - texTile.top();
        canvas.drawBitmap(tileSrc, drawX, drawY);

        SkPaint paint;
        setupPaint(paint);
        canvas.drawBitmap(tileSrc, drawX, drawY, &paint);
    }

    renderTools.swap();
}
