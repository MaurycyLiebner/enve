#ifndef GPURASTEREFFECT_H
#define GPURASTEREFFECT_H
#include "Animators/staticcomplexanimator.h"
#include "shadereffectcreator.h"
class GPUEffectAnimators;

enum class GpuEffectType : short {
    BLUR,
    SHADOW,
    CUSTOM, // C++
    CUSTOM_SHADER, // xml, GLSL
    TYPE_COUNT
};

class GPURasterEffectCaller : public StdSelfRef {
public:
    virtual void render(QGL33c * const gl,
                        GpuRenderTools& renderTools,
                        GpuRenderData& data) = 0;
};


class GpuEffect : public StaticComplexAnimator {
    friend class SelfRef;
protected:
    GpuEffect(const QString &name, const GpuEffectType type);
public:
    virtual stdsptr<GPURasterEffectCaller>
        getEffectCaller(const qreal relFrame) const = 0;
    virtual QMarginsF getMarginAtRelFrame(const qreal frame) {
        Q_UNUSED(frame);
        return QMarginsF();
    }
    virtual void writeIdentifier(QIODevice * const dst) const;

    bool SWT_isGpuEffect() const { return true; }

    bool isVisible() const { return true; }
private:
    const GpuEffectType mType;
};

#endif // GPURASTEREFFECT_H
