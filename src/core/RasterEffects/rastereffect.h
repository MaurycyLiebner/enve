#ifndef RASTEREFFECT_H
#define RASTEREFFECT_H
#include "../Animators/eeffect.h"
#include "../glhelpers.h"

class RasterEffectCaller;

enum class RasterEffectType : short {
    BLUR,
    SHADOW,
    CUSTOM, // C++
    CUSTOM_SHADER, // xml, GLSL
    TYPE_COUNT
};

class RasterEffect : public eEffect {
    e_OBJECT
    Q_OBJECT
protected:
    RasterEffect(const QString &name,
                 const HardwareSupport hwSupport,
                 const bool hwInterchangeable,
                 const RasterEffectType type);
public:
    virtual stdsptr<RasterEffectCaller> getEffectCaller(const qreal relFrame) const = 0;

    virtual bool forceMargin() const { return false; }

    void writeIdentifier(QIODevice * const dst) const;

    void setupTreeViewMenu(PropertyMenu * const menu);

    bool SWT_isRasterEffect() const final { return true; }

    QMimeData *SWT_createMimeData() final;

    HardwareSupport instanceHwSupport() const {
        return mInstHwSupport;
    }

    void switchInstanceHwSupport() {
        if(mTypeHwSupport == HardwareSupport::cpuOnly) return;
        if(mTypeHwSupport == HardwareSupport::gpuOnly) return;
        if(mInstHwSupport == HardwareSupport::cpuOnly) {
            if(mHwInterchangeable) mInstHwSupport = mTypeHwSupport;
            else mInstHwSupport = HardwareSupport::gpuOnly;
        } else if(mInstHwSupport == HardwareSupport::gpuOnly) {
            mInstHwSupport = HardwareSupport::cpuOnly;
        } else mInstHwSupport = HardwareSupport::gpuOnly;
        if(!mHwInterchangeable) prp_afterWholeInfluenceRangeChanged();
    }
private:
    const RasterEffectType mType;
    const HardwareSupport mTypeHwSupport;
    const bool mHwInterchangeable;
    HardwareSupport mInstHwSupport;
};

#endif // RASTEREFFECT_H
