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

    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame) {
        if(mRandomize->getValue()) {
            if(mSmoothTransform->getValue()) {
                *firstIdentical = relFrame;
                *lastIdentical = relFrame;
            } else {
                int frameStep = mRandomizeStep->getCurrentIntValueAtRelFrame(relFrame);
                *firstIdentical = relFrame - relFrame % frameStep;
                *lastIdentical = *firstIdentical + frameStep;
            }
        } else {
            PathEffect::prp_getFirstAndLastIdenticalRelFrame(firstIdentical,
                                                             lastIdentical,
                                                             relFrame);
        }
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

    void prp_setAbsFrame(const int &frame) {
        ComplexAnimator::prp_setAbsFrame(frame);
        if(mRandomize->getValue()) {
            prp_callUpdater();
        }
    }
private:
    uint32_t mSeedAssist = 0;

    QrealAnimatorQSPtr mSegLength;
    QrealAnimatorQSPtr mMaxDev;
    QrealAnimatorQSPtr mSmoothness;
    BoolPropertyContainerQSPtr mRandomize;
    IntAnimatorQSPtr mRandomizeStep;
    BoolPropertyContainerQSPtr mSmoothTransform;
    QrealAnimatorQSPtr mEasing;
    IntAnimatorQSPtr mSeed;
    BoolPropertyQSPtr mRepeat;
};

#endif // DISPLACEPATHEFFECT_H
