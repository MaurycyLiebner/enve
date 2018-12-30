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
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);

    FrameRange prp_getFirstAndLastIdenticalRelFrame(const int &relFrame) {
        if(mRandomize->getValue()) {
            if(mSmoothTransform->getValue()) {
                return {relFrame, relFrame};
            } else {
                int frameStep = mRandomizeStep->getCurrentIntValueAtRelFrame(relFrame);
                int min = relFrame - relFrame % frameStep;
                return {min, min + frameStep};
            }
        }
        return PathEffect::prp_getFirstAndLastIdenticalRelFrame(relFrame);
    }

    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2) {
        if(mRandomize->getValue()) {
            int frameStep1 = mRandomizeStep->getCurrentIntValueAtRelFrame(relFrame1);
            int frameStep2 = mRandomizeStep->getCurrentIntValueAtRelFrame(relFrame2);
            return relFrame1 - relFrame1 % frameStep1 ==
                    relFrame2 - relFrame2 % frameStep2;
        }
        return PathEffect::prp_differencesBetweenRelFrames(relFrame1,
                                                           relFrame2);
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
