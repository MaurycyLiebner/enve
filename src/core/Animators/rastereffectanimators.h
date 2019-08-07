#ifndef RasterEffectANIMATORS_H
#define RasterEffectANIMATORS_H
#include "Animators/dynamiccomplexanimator.h"
#include "smartPointers/ememory.h"
#include "RasterEffects/rastereffect.h"
#include "ShaderEffects/shadereffect.h"

class BoundingBox;
struct BoxRenderData;
qsptr<RasterEffect> readIdCreateRasterEffect(QIODevice * const src);
typedef DynamicComplexAnimator<
    RasterEffect, &RasterEffect::writeIdentifier,
    &readIdCreateRasterEffect> RasterEffectAnimatorsBase;
class RasterEffectAnimators : public RasterEffectAnimatorsBase {
    e_OBJECT
protected:
    RasterEffectAnimators(BoundingBox * const parentBox);
public:
    bool SWT_isRasterEffectAnimators() const { return true; }

    BoundingBox *getParentBox() { return mParentBox_k; }
    bool hasEffects();
    QMargins getEffectsMargin(const qreal relFrame) const;
    bool unbound() const;

    void addEffects(const qreal relFrame,
                    BoxRenderData * const data);

    void updateIfUsesProgram(const ShaderEffectProgram * const program);
    //void readRasterEffect(QIODevice *target);
private:
    void updateUnbound();

    bool mUnbound = false;
    BoundingBox * const mParentBox_k;
};

#endif // RasterEffectANIMATORS_H
