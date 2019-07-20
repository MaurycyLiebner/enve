#ifndef EXAMPLEGPUEFFECT_H
#define EXAMPLEGPUEFFECT_H

#include "examplegpueffect_global.h"

class ExampleGpuEffectCaller000 : public GPURasterEffectCaller {
public:
    ExampleGpuEffectCaller000(const qreal radius) :
        GPURasterEffectCaller(true, QMargins() + qCeil(radius)),
        mRadius(static_cast<float>(radius)) {}

    void render(QGL33 * const gl,
                GpuRenderTools& renderTools,
                GpuRenderData& data);
private:
    const float mRadius;
};

class ExampleGpuEffect000 : public CustomGpuEffect {
public:
    ExampleGpuEffect000();

    stdsptr<GPURasterEffectCaller>
            getEffectCaller(const qreal relFrame) const;
    QMargins getMarginAtRelFrame(const qreal frame) const;
    bool forceWholeBase() const { return true; }

    CustomIdentifier getIdentifier() const;
private:
    qsptr<QrealAnimator> mRadius;
};

#endif // EXAMPLEGPUEFFECT_H
