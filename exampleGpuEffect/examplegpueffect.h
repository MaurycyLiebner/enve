#ifndef EXAMPLEGPUEFFECT_H
#define EXAMPLEGPUEFFECT_H

#include "examplegpueffect_global.h"

class ExampleGpuEffectCaller000 : public GPURasterEffectCaller {
public:
    ExampleGpuEffectCaller000(const SkScalar radius) :
        mSigma(radius/3) {}

    void render(QGL33c * const gl,
                GpuRenderTools& renderTools,
                GpuRenderData& data);
private:
    const SkScalar mSigma;
};

class ExampleGpuEffect000 : public CustomGpuEffect {
public:
    ExampleGpuEffect000();

    stdsptr<GPURasterEffectCaller>
            getEffectCaller(const qreal relFrame) const;
    QMarginsF getMarginAtRelFrame(const qreal frame);

    CustomIdentifier getIdentifier() const;
private:
    qsptr<QrealAnimator> mRadius;
};

#endif // EXAMPLEGPUEFFECT_H
