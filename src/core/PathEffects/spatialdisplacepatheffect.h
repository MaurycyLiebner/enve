#ifndef SPATIALDISPLACEPATHEFFECT_H
#define SPATIALDISPLACEPATHEFFECT_H
#include "PathEffects/patheffect.h"
class IntAnimator;
class RandomGrid;
class BoolPropertyContainer;

class SpatialDisplacePathEffect : public PathEffect {
    e_OBJECT
protected:
    SpatialDisplacePathEffect();
public:
    void apply(const qreal relFrame, const SkPath &src,
               SkPath * const dst);
private:
    qsptr<QrealAnimator> mMaxDev;
    qsptr<BoolPropertyContainer> mLengthBased;
    qsptr<RandomGrid> mSeed;

    qsptr<QrealAnimator> mSegLength;
    qsptr<QrealAnimator> mSmoothness;
};

#endif // SPATIALDISPLACEPATHEFFECT_H
