#ifndef PATHEFFECTANIMATORS_H
#define PATHEFFECTANIMATORS_H
#include "PathEffects/patheffect.h"
#include "Animators/dynamiccomplexanimator.h"
#include "skia/skiaincludes.h"
class PathBox;
class BoundingBox;

qsptr<PathEffect> readIdCreatePathEffect(QIODevice * const src);

typedef DynamicComplexAnimator<PathEffect,
        &PathEffect::writeIdentifier,
        &readIdCreatePathEffect> PathEffectAnimatorsBase;
class PathEffectAnimators : public PathEffectAnimatorsBase {
    friend class SelfRef;
protected:
    PathEffectAnimators(BoundingBox * const parentPath);
public:
    bool SWT_isPathEffectAnimators() const;

    void addEffect(const qsptr<PathEffect> &effect);
    bool hasEffects();

    void apply(const qreal relFrame, SkPath * const srcDstPath);

    void removeEffect(const qsptr<PathEffect>& effect);
    BoundingBox *getParentBox();
    void readPathEffect(QIODevice * const target);
private:
    qptr<BoundingBox> mParentBox;
};


#endif // PATHEFFECTANIMATORS_H
