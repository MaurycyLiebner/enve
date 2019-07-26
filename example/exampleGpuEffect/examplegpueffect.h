#ifndef EXAMPLEGPUEFFECT_H
#define EXAMPLEGPUEFFECT_H

#include "examplegpueffect_global.h"

class ExampleGpuEffectCaller000 : public RasterEffectCaller {
public:
    ExampleGpuEffectCaller000(const qreal radius) :
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

class ExampleGpuEffect000 : public CustomGpuEffect {
public:
    ExampleGpuEffect000();

    stdsptr<RasterEffectCaller>
            getEffectCaller(const qreal relFrame) const;
    QMargins getMarginAtRelFrame(const qreal frame) const;
    bool forceWholeBase() const { return true; }

    CustomIdentifier getIdentifier() const;
private:
    qsptr<QrealAnimator> mRadius;
};

#endif // EXAMPLEGPUEFFECT_H
