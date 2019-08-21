#ifndef ESHADOW_H
#define ESHADOW_H

#include "eshadow_global.h"
#include "enveCore/skia/skqtconversions.h"
#include "enveCore/Animators/coloranimator.h"
#include "enveCore/Animators/qpointfanimator.h"

class eShadowCaller : public RasterEffectCaller {
public:
    eShadowCaller(const HardwareSupport hwSupport,
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

class eShadow : public CustomRasterEffect {
public:
    eShadow();

    stdsptr<RasterEffectCaller> getEffectCaller(const qreal relFrame) const;
    bool forceMargin() const { return true; }

    CustomIdentifier getIdentifier() const;
private:
    qsptr<QrealAnimator> mBlurRadius;
    qsptr<QrealAnimator> mOpacity;
    qsptr<ColorAnimator> mColor;
    qsptr<QPointFAnimator> mTranslation;
};

#endif // ESHADOW_H
