#ifndef DISPLACEPATHEFFECT_H
#define DISPLACEPATHEFFECT_H
#include "PathEffects/patheffect.h"
class IntAnimator;
class BoolPropertyContainer;

class DisplacePathEffect : public PathEffect {
    friend class SelfRef;
protected:
    DisplacePathEffect(const bool &outlinePathEffect);
public:
    void apply(const qreal &relFrame,
               const SkPath &src,
               SkPath * const dst);
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);

    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const;
private:
    uint32_t mSeedAssist = 0;

    qsptr<QrealAnimator> mSegLength;
    qsptr<QrealAnimator> mMaxDev;
    qsptr<QrealAnimator> mSmoothness;
    qsptr<BoolPropertyContainer> mRandomize;
    qsptr<IntAnimator> mRandomizeStep;
    qsptr<BoolPropertyContainer> mSmoothTransform;
    qsptr<QrealAnimator> mEasing;
    qsptr<IntAnimator> mSeed;
    qsptr<BoolProperty> mRepeat;
};

#endif // DISPLACEPATHEFFECT_H
