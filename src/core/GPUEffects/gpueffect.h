#ifndef GPURASTEREFFECT_H
#define GPURASTEREFFECT_H
#include "../Animators/staticcomplexanimator.h"
#include "../glhelpers.h"

enum class GpuEffectType : short {
    BLUR,
    SHADOW,
    CUSTOM, // C++
    CUSTOM_SHADER, // xml, GLSL
    TYPE_COUNT
};

class RasterEffectCaller : public StdSelfRef {
protected:
    RasterEffectCaller() : RasterEffectCaller(false, QMargins()) {}
    RasterEffectCaller(const bool forceMargin,
                       const QMargins& margin) :
        fForceMargin(forceMargin), fMargin(margin) {}

public:
    virtual void processGpu(QGL33 * const gl,
                            GpuRenderTools& renderTools,
                            GpuRenderData& data) {
        Q_UNUSED(gl);
        Q_UNUSED(renderTools);
        Q_UNUSED(data);
    }

    virtual void processCpu(CpuRenderTools& renderTools,
                            const CpuRenderData& data) {
        Q_UNUSED(renderTools);
        Q_UNUSED(data);
    }

    virtual HardwareSupport hardwareSupport() const = 0;
    bool gpuOnly() const { return hardwareSupport() == HardwareSupport::GPU_ONLY; }
    bool cpuOnly() const { return hardwareSupport() == HardwareSupport::CPU_ONLY; }

    void setSrcRect(const SkIRect& srcRect,
                    const SkIRect& clampRect,
                    const bool canIgnoreClamp) {
        if(fForceMargin && canIgnoreClamp) {
            fSrcRect = SkIRect::MakeLTRB(srcRect.left() - fMargin.left(),
                                         srcRect.top() - fMargin.top(),
                                         srcRect.right() + fMargin.right(),
                                         srcRect.bottom() + fMargin.bottom());
            fDstRect = fSrcRect;
        } else {
            fSrcRect = srcRect;
            fDstRect = SkIRect::MakeLTRB(
                        qMax(srcRect.left() - fMargin.left(), clampRect.left()),
                        qMax(srcRect.top() - fMargin.top(), clampRect.top()),
                        qMin(srcRect.right() + fMargin.right(), clampRect.right()),
                        qMin(srcRect.bottom() + fMargin.bottom(), clampRect.bottom()));
        }
    }

    const SkIRect& getDstRect() const { return  fDstRect; }
protected:
    const bool fForceMargin;
    const QMargins fMargin;
    SkIRect fSrcRect;
    SkIRect fDstRect;
};

enum class HardwareSupport : short;
class GpuEffect : public StaticComplexAnimator {
    friend class SelfRef;
protected:
    GpuEffect(const QString &name, const GpuEffectType type);
public:
    virtual stdsptr<RasterEffectCaller>
        getEffectCaller(const qreal relFrame) const = 0;

    virtual QMargins getMarginAtRelFrame(const qreal frame) const {
        Q_UNUSED(frame);
        return QMargins();
    }
    virtual bool forceWholeBase() const { return false; }
    virtual void writeIdentifier(QIODevice * const dst) const;

    bool SWT_isGpuEffect() const { return true; }

    bool isVisible() const { return true; }
private:
    const GpuEffectType mType;
};

#endif // GPURASTEREFFECT_H
