#ifndef GPURASTEREFFECT_H
#define GPURASTEREFFECT_H
#include "../Animators/eeffect.h"
#include "../glhelpers.h"

enum class GpuEffectType : short {
    BLUR,
    SHADOW,
    CUSTOM, // C++
    CUSTOM_SHADER, // xml, GLSL
    TYPE_COUNT
};
#define qMax3(a, b, c) qMax(a, qMax(b, c))
#define qMin3(a, b, c) qMin(a, qMin(b, c))

class RasterEffectCaller : public StdSelfRef {
protected:
    RasterEffectCaller() : RasterEffectCaller(false, QMargins()) {}
    RasterEffectCaller(const bool forceMargin,
                       const QMargins& margin) :
        fForceMargin(forceMargin), fMargin(margin) {}

public:
    virtual HardwareSupport hardwareSupport() const = 0;

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

    virtual int cpuThreads(const int available, const int area) const {
        return qMin(area/(150*150) + 1, available);
    }

    bool gpuOnly() const { return hardwareSupport() == HardwareSupport::GPU_ONLY; }
    bool cpuOnly() const { return hardwareSupport() == HardwareSupport::CPU_ONLY; }

    void setSrcRect(const SkIRect& srcRect,
                    const SkIRect& baseRect,
                    const SkIRect& clampRect) {
        if(fForceMargin) {
            fSrcRect = SkIRect::MakeLTRB(qMax(srcRect.left() - fMargin.left(),
                                              baseRect.left()),
                                         qMax(srcRect.top() - fMargin.top(),
                                              baseRect.top()),
                                         qMin(srcRect.right() + fMargin.right(),
                                              baseRect.right()),
                                         qMin(srcRect.bottom() + fMargin.bottom(),
                                              baseRect.bottom()));
            fDstRect = fSrcRect;
        } else {
            fSrcRect = srcRect;
            fDstRect = SkIRect::MakeLTRB(
                        qMax3(srcRect.left() - fMargin.left(),
                              clampRect.left(), baseRect.left()),
                        qMax3(srcRect.top() - fMargin.top(),
                              clampRect.top(), baseRect.top()),
                        qMin3(srcRect.right() + fMargin.right(),
                              clampRect.right(), baseRect.right()),
                        qMin3(srcRect.bottom() + fMargin.bottom(),
                              clampRect.bottom(), baseRect.bottom()));
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
class GpuEffect : public eEffect {
    friend class SelfRef;
    Q_OBJECT
protected:
    GpuEffect(const QString &name, const GpuEffectType type);
public:
    virtual stdsptr<RasterEffectCaller>
        getEffectCaller(const qreal relFrame) const = 0;

    virtual QMargins getMarginAtRelFrame(const qreal frame) const {
        Q_UNUSED(frame);
        return QMargins();
    }
    virtual bool forceMargin() const { return false; }
    void writeIdentifier(QIODevice * const dst) const;

    bool SWT_isGpuEffect() const { return true; }
private:
    const GpuEffectType mType;
};

#endif // GPURASTEREFFECT_H
