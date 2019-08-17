#ifndef RASTEREFFECTCALLER_H
#define RASTEREFFECTCALLER_H
#include "../smartPointers/stdselfref.h"
#include "../glhelpers.h"

enum class HardwareSupport : short;

class RasterEffectCaller : public StdSelfRef {
    e_OBJECT
public:
    RasterEffectCaller(const HardwareSupport hwSupport);
    RasterEffectCaller(const HardwareSupport hwSupport,
                       const bool forceMargin, const QMargins& margin);

    HardwareSupport hardwareSupport() const {
        return fHwSupport;
    }

    virtual void processGpu(QGL33 * const gl,
                            GpuRenderTools& renderTools,
                            GpuRenderData& data);

    virtual void processCpu(CpuRenderTools& renderTools,
                            const CpuRenderData& data);

    virtual int cpuThreads(const int available, const int area) const;

    bool interchangeable() const {
        return fHwSupport != HardwareSupport::cpuOnly &&
               fHwSupport != HardwareSupport::gpuOnly;
    }

    void setSrcRect(const SkIRect& srcRect, const SkIRect& clampRect);

    const SkIRect& getDstRect() const { return  fDstRect; }
protected:
    const bool fForceMargin;
    const HardwareSupport fHwSupport;
    const QMargins fMargin;
    SkIRect fSrcRect;
    SkIRect fDstRect;
};

#endif // RASTEREFFECTCALLER_H
