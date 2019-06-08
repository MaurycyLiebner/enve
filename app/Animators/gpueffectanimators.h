#ifndef GPUEFFECTANIMATORS_H
#define GPUEFFECTANIMATORS_H
#include "Animators/dynamiccomplexanimator.h"
#include "smartPointers/sharedpointerdefs.h"
#include "GPUEffects/gpurastereffect.h"

class BoundingBox;
struct BoundingBoxRenderData;
qsptr<GPURasterEffect> readIdCreateGPURasterEffect(QIODevice * const src);
typedef DynamicComplexAnimator<
    GPURasterEffect, &GPURasterEffect::writeIdentifier,
    &readIdCreateGPURasterEffect> GPUEffectAnimatorsBase;
class GPUEffectAnimators : public GPUEffectAnimatorsBase {
    friend class SelfRef;
protected:
    GPUEffectAnimators(BoundingBox *parentBox);
public:
    bool SWT_isRasterGPUEffectAnimators() const { return true; }

    qreal getEffectsMargin() const;

    void setParentBox(BoundingBox *box);
    BoundingBox *getParentBox() { return mParentBox_k; }

    bool hasEffects();

    qreal getEffectsMarginAtRelFrame(const int relFrame) const;
    qreal getEffectsMarginAtRelFrameF(const qreal relFrame) const;

    void addEffectRenderDataToListF(const qreal relFrame,
                                    BoundingBoxRenderData * const data);

    void updateIfUsesProgram(const GPURasterEffectProgram * const program);
    //void readPixmapEffect(QIODevice *target);
private:
    BoundingBox * const mParentBox_k;
};

#endif // GPUEFFECTANIMATORS_H
