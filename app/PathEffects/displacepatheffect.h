#ifndef DISPLACEPATHEFFECT_H
#define DISPLACEPATHEFFECT_H
#include "patheffect.h"

class DisplacePathEffect : public PathEffect {
    Q_OBJECT
    friend class SelfRef;
public:
    DisplacePathEffect(const bool &outlinePathEffect);

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src,
                               SkPath *dst,
                               const qreal &scale = 1.,
                               const bool & = false);
    void filterPathForRelFrameF(const qreal &relFrame,
                                const SkPath &src,
                                SkPath *dst,
                                const bool &);
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);

    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const {
        if(mRandomize->getValue()) {
            if(mSmoothTransform->getValue()) {
                return {relFrame, relFrame};
            } else {
                int frameStep = mRandomizeStep->getCurrentIntValueAtRelFrame(relFrame);
                int min = relFrame - relFrame % frameStep;
                return {min, min + frameStep};
            }
        }
        return PathEffect::prp_getIdenticalRelFrameRange(relFrame);
    }
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
