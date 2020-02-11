#ifndef BLENDEFFECT_H
#define BLENDEFFECT_H

#include "Animators/staticcomplexanimator.h"

#include "Properties/boxtargetproperty.h"

class PathBox;
class ChildRenderData;

enum class BlendEffectType {
    move, targeted
};

class BlendEffect : public StaticComplexAnimator {
    Q_OBJECT
    e_OBJECT
protected:
    BlendEffect(const QString &name,
                const BlendEffectType type);
public:
    virtual void blendSetup(ChildRenderData &data,
                            const int index,
                            const qreal relFrame,
                            QList<ChildRenderData> &delayed) const = 0;

    using Delayed = std::function<bool(int id,
                                       BoundingBox* prev,
                                       BoundingBox* next)>;
    virtual void detachedBlendSetup(BoundingBox* const boxToDraw,
                                const qreal relFrame,
                                SkCanvas * const canvas,
                                const SkFilterQuality filter,
                                const int drawId,
                                QList<Delayed> &delayed) const = 0;
    virtual void drawBlendSetup(const qreal relFrame,
                                SkCanvas * const canvas) const = 0;

    void prp_setupTreeViewMenu(PropertyMenu * const menu);

    void writeIdentifier(eWriteStream &dst) const;

    bool isPathValid() const;
    SkPath clipPath(const qreal relFrame) const;
private:
    PathBox* clipPathSource() const;

    ConnContextQPtr<BoundingBox> mClipBox;
    const BlendEffectType mType;
    qsptr<BoxTargetProperty> mClipPath;
};

#endif // BLENDEFFECT_H
