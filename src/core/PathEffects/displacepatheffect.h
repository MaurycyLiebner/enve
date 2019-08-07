#ifndef DISPLACEPATHEFFECT_H
#define DISPLACEPATHEFFECT_H
#include "PathEffects/patheffect.h"
class IntAnimator;
class BoolPropertyContainer;

class DisplacePathEffect : public PathEffect {
    e_OBJECT
protected:
    DisplacePathEffect();
public:
    void apply(const qreal relFrame, const SkPath &src,
               SkPath * const dst);
private:    
    qsptr<QrealAnimator> mSeed;
    qsptr<QrealAnimator> mMaxDev;
    qsptr<BoolPropertyContainer> mLengthBased;

    qsptr<QrealAnimator> mSegLength;
    qsptr<QrealAnimator> mSmoothness;
};

#endif // DISPLACEPATHEFFECT_H
