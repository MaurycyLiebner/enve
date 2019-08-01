#ifndef EXAMPLERASTEREFFECT_H
#define EXAMPLERASTEREFFECT_H

#include "examplerastereffect_global.h"

class ExampleRasterEffectCaller000 : public RasterEffectCaller {
public:
    ExampleRasterEffectCaller000(const qreal radius) :
        RasterEffectCaller(true, QMargins() + qCeil(radius)),
        mRadius(static_cast<float>(radius)) {}

    void processGpu(QGL33 * const gl,
                    GpuRenderTools& renderTools,
                    GpuRenderData& data);
    void processCpu(CpuRenderTools& renderTools,
                    const CpuRenderData &data);

    HardwareSupport hardwareSupport() const {
        return HardwareSupport::GPU_PREFFERED;
    }
private:
    const float mRadius;
};

class ExampleRasterEffect000 : public CustomRasterEffect {
public:
    ExampleRasterEffect000();

    stdsptr<RasterEffectCaller>
            getEffectCaller(const qreal relFrame) const;
    QMargins getMarginAtRelFrame(const qreal frame) const;
    bool forceMargin() const { return true; }

    CustomIdentifier getIdentifier() const;
private:
    qsptr<QrealAnimator> mRadius;
};

#endif // EXAMPLERASTEREFFECT_H
