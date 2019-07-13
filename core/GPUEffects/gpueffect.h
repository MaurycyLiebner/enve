#ifndef GPURASTEREFFECT_H
#define GPURASTEREFFECT_H
#include "Animators/staticcomplexanimator.h"
#include "glhelpers.h"

enum class GpuEffectType : short {
    BLUR,
    SHADOW,
    CUSTOM, // C++
    CUSTOM_SHADER, // xml, GLSL
    TYPE_COUNT
};

class GPURasterEffectCaller : public StdSelfRef {
protected:
    GPURasterEffectCaller(const bool forceMargin, const QMargins& margin) :
        fForceMargin(forceMargin), fMargin(margin) {}
public:
    virtual void render(QGL33c * const gl,
                        GpuRenderTools& renderTools,
                        GpuRenderData& data) = 0;

    SkIRect setSrcRectUpdateDstRect(const SkIRect& srcRect,
                                    const SkIRect& clampRect) {
        if(fForceMargin) {
            fDstRect = SkIRect::MakeLTRB(srcRect.left() - fMargin.left(),
                                         srcRect.top() - fMargin.top(),
                                         srcRect.right() + fMargin.right(),
                                         srcRect.bottom() + fMargin.bottom());
            fSrcRect = fDstRect;
        } else {
            fSrcRect = srcRect;
            fDstRect = SkIRect::MakeLTRB(
                        qMax(srcRect.left() - fMargin.left(), clampRect.left()),
                        qMax(srcRect.top() - fMargin.top(), clampRect.top()),
                        qMin(srcRect.right() + fMargin.right(), clampRect.right()),
                        qMin(srcRect.bottom() + fMargin.bottom(), clampRect.bottom()));
        }

        return fDstRect;
    }

    const SkIRect& getDstRect() const { return  fDstRect; }
protected:
    const bool fForceMargin;
    const QMargins fMargin;
    SkIRect fSrcRect;
    SkIRect fDstRect;
};

class GpuEffect : public StaticComplexAnimator {
    friend class SelfRef;
protected:
    GpuEffect(const QString &name, const GpuEffectType type);
public:
    virtual stdsptr<GPURasterEffectCaller>
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
