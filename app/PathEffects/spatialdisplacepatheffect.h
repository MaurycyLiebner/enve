#ifndef SPATIALDISPLACEPATHEFFECT_H
#define SPATIALDISPLACEPATHEFFECT_H
#include "PathEffects/patheffect.h"
class IntAnimator;
class RandomGrid;
class BoolPropertyContainer;

class SpatialDisplacePathEffect : public PathEffect {
    friend class SelfRef;
protected:
    SpatialDisplacePathEffect();
public:
    void apply(const qreal relFrame, const SkPath &src,
               SkPath * const dst);
    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);
private:
    qsptr<QrealAnimator> mMaxDev;
    qsptr<BoolPropertyContainer> mLengthBased;
    qsptr<RandomGrid> mSeed;

    qsptr<QrealAnimator> mSegLength;
    qsptr<QrealAnimator> mSmoothness;
};

#endif // SPATIALDISPLACEPATHEFFECT_H
