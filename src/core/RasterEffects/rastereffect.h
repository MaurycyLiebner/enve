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

enum class HardwareSupport : short;
class RasterEffect : public eEffect {
    e_OBJECT
    Q_OBJECT
protected:
    RasterEffect(const QString &name, const RasterEffectType type);
public:
    virtual stdsptr<RasterEffectCaller>
        getEffectCaller(const qreal relFrame) const = 0;

    virtual QMargins getMarginAtRelFrame(const qreal frame) const {
        Q_UNUSED(frame);
        return QMargins();
    }
    virtual bool forceMargin() const { return false; }

    void writeIdentifier(QIODevice * const dst) const;

    void setupTreeViewMenu(PropertyMenu * const menu);

    bool SWT_isRasterEffect() const { return true; }

    QMimeData *SWT_createMimeData() final;
private:
    const RasterEffectType mType;
};

#endif // RASTEREFFECT_H
