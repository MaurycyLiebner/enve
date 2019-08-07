#ifndef RASTEREFFECTCALLER_H
#define RASTEREFFECTCALLER_H
#include "../smartPointers/stdselfref.h"
#include "../glhelpers.h"

enum class HardwareSupport : short;

#define qMax3(a, b, c) qMax(a, qMax(b, c))
#define qMin3(a, b, c) qMin(a, qMin(b, c))

class RasterEffectCaller : public StdSelfRef {
    e_OBJECT
public:
    RasterEffectCaller() : RasterEffectCaller(false, QMargins()) {}
    RasterEffectCaller(const bool forceMargin, const QMargins& margin);

    virtual HardwareSupport hardwareSupport() const = 0;

    virtual void processGpu(QGL33 * const gl,
                            GpuRenderTools& renderTools,
                            GpuRenderData& data);

    virtual void processCpu(CpuRenderTools& renderTools,
                            const CpuRenderData& data);

    virtual int cpuThreads(const int available, const int area) const;

    bool gpuOnly() const
    { return hardwareSupport() == HardwareSupport::gpuOnly; }
    bool cpuOnly() const
    { return hardwareSupport() == HardwareSupport::cpuOnly; }

    void setSrcRect(const SkIRect& srcRect, const SkIRect& clampRect);

    const SkIRect& getDstRect() const { return  fDstRect; }
protected:
    const bool fForceMargin;
    const QMargins fMargin;
    SkIRect fSrcRect;
    SkIRect fDstRect;
};

#endif // RASTEREFFECTCALLER_H
