#ifndef SPATIALDISPLACEPATHEFFECT_H
#define SPATIALDISPLACEPATHEFFECT_H
#include "PathEffects/patheffect.h"
class IntAnimator;
class RandomGrid;
class BoolPropertyContainer;

class SpatialDisplacePathEffect : public PathEffect {
    friend class SelfRef;
protected:
    SpatialDisplacePathEffect(const bool &outlinePathEffect);
public:
    void apply(const qreal &relFrame, const SkPath &src,
               SkPath * const dst);
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
private:
    qsptr<QrealAnimator> mMaxDev;
    qsptr<BoolPropertyContainer> mLengthBased;
    qsptr<RandomGrid> mSeed;

    qsptr<QrealAnimator> mSegLength;
    qsptr<QrealAnimator> mSmoothness;
};

#endif // SPATIALDISPLACEPATHEFFECT_H
