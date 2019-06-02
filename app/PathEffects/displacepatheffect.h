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
    void apply(const qreal &relFrame, const SkPath &src,
               SkPath * const dst);
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
private:    
    qsptr<QrealAnimator> mSeed;
    qsptr<QrealAnimator> mMaxDev;
    qsptr<BoolPropertyContainer> mLengthBased;

    qsptr<QrealAnimator> mSegLength;
    qsptr<QrealAnimator> mSmoothness;
};

#endif // DISPLACEPATHEFFECT_H
