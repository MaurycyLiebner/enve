#ifndef EBLUR_H
#define EBLUR_H

#include "eblur_global.h"

class eBlurCaller : public RasterEffectCaller {
public:
    eBlurCaller(const HardwareSupport hwSupport,
                                 const qreal radius) :
        RasterEffectCaller(hwSupport, true, QMargins() + qCeil(radius)),
        mRadius(static_cast<float>(radius)) {}

    void processGpu(QGL33 * const gl,
                    GpuRenderTools& renderTools,
                    GpuRenderData& data);
    void processCpu(CpuRenderTools& renderTools,
                    const CpuRenderData &data);
private:
    const float mRadius;
};

class eBlur : public CustomRasterEffect {
public:
    eBlur();

    stdsptr<RasterEffectCaller> getEffectCaller(const qreal relFrame) const;
    bool forceMargin() const { return true; }

    CustomIdentifier getIdentifier() const;
private:
    qsptr<QrealAnimator> mRadius;
};

#endif // EBLUR_H
