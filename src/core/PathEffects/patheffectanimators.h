#ifndef PATHEFFECTANIMATORS_H
#define PATHEFFECTANIMATORS_H
#include "PathEffects/patheffect.h"
#include "Animators/dynamiccomplexanimator.h"
#include "skia/skiaincludes.h"
class PathBox;
class BoundingBox;

qsptr<PathEffect> readIdCreatePathEffect(eReadStream& src);

typedef DynamicComplexAnimator<PathEffect,
        &PathEffect::writeIdentifier,
        &readIdCreatePathEffect> PathEffectAnimatorsBase;
class PathEffectAnimators : public PathEffectAnimatorsBase {
    e_OBJECT
protected:
    PathEffectAnimators();
public:
    bool SWT_isPathEffectAnimators() const;

    bool hasEffects();
    void apply(const qreal relFrame, SkPath * const srcDstPath);
    void readPathEffect(eReadStream &target);
private:
};


#endif // PATHEFFECTANIMATORS_H
