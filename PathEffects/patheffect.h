#ifndef PATHEFFECT_H
#define PATHEFFECT_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimator.h"
#include "Animators/qpointfanimator.h"
#include "Animators/intanimator.h"
#include "Properties/boolproperty.h"
#include "Animators/intanimator.h"
#include "skiaincludes.h"

enum PathEffectType : short {
    DISPLACE_PATH_EFFECT,
    DASH_PATH_EFFECT,
    DUPLICATE_PATH_EFFECT,
    SUM_PATH_EFFECT
};

class PathEffect : public ComplexAnimator {
    Q_OBJECT
public:
    PathEffect(const PathEffectType &type,
               const bool &outlinePathEffect);

    virtual Property *makeDuplicate() = 0;
    virtual void makeDuplicate(Property *target) = 0;
    virtual void filterPath(const SkPath &, SkPath *) = 0;
    virtual void filterPathForRelFrame(const int &,
                                       const SkPath &,
                                       SkPath *) = 0;
    virtual void writePathEffect(QFile *file);
    bool applyBeforeThickness() {
        if(mApplyBeforeThickness == NULL) return false;
        return mApplyBeforeThickness->getValue();
    }
protected:
    bool mOutlineEffect;
    QSharedPointer<BoolProperty> mApplyBeforeThickness;
    PathEffectType mPathEffectType;
};

class DisplacePathEffect : public PathEffect {
    Q_OBJECT
public:
    DisplacePathEffect(const bool &outlinePathEffect);

    Property *makeDuplicate();

    void makeDuplicate(Property *target);

    void duplicateAnimatorsFrom(QrealAnimator *segLen,
                                QrealAnimator *maxDev);

    void filterPath(const SkPath &src, SkPath *dst);
    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src, SkPath *dst);
    void writePathEffect(QFile *file);
    void readDisplacePathEffect(QFile *file);

    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame) {
        if(mRandomize->getValue()) {
            int frameStep = mRandomizeStep->getCurrentIntValueAtRelFrame(relFrame);
            *firstIdentical = relFrame - relFrame % frameStep;
            *lastIdentical = *firstIdentical + frameStep;
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
    QSharedPointer<QrealAnimator> mSegLength =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mMaxDev =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mSmoothness =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<BoolProperty> mRandomize =
            (new BoolProperty())->ref<BoolProperty>();
    QSharedPointer<IntAnimator> mRandomizeStep =
            (new IntAnimator())->ref<IntAnimator>();
    QSharedPointer<IntAnimator> mSeed =
            (new IntAnimator())->ref<IntAnimator>();
    uint32_t mSeedAssist = 0;
};

class DuplicatePathEffect : public PathEffect {
    Q_OBJECT
public:
    DuplicatePathEffect(const bool &outlinePathEffect);

    Property *makeDuplicate();

    void makeDuplicate(Property *target);

    void duplicateAnimatorsFrom(QPointFAnimator *trans);

    void filterPath(const SkPath &src, SkPath *dst);

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src, SkPath *dst);
    void writePathEffect(QFile *file);
    void readDuplicatePathEffect(QFile *file);
private:
    QSharedPointer<QPointFAnimator> mTranslation =
            (new QPointFAnimator())->ref<QPointFAnimator>();
};

class PathBox;
#include "Properties/boxtargetproperty.h"
class SumPathEffect : public PathEffect {
    Q_OBJECT
public:
    SumPathEffect(PathBox *parentPath,
                  const bool &outlinePathEffect);

    Property *makeDuplicate() {}

    void makeDuplicate(Property *target) {}

    void filterPath(const SkPath &src, SkPath *dst) {}

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src, SkPath *dst);

    void writePathEffect(QFile *file);
    void readSumPathEffect(QFile *file);
private:
    PathBox *mParentPathBox;
    QSharedPointer<BoxTargetProperty> mBoxTarget =
            (new BoxTargetProperty())->ref<BoxTargetProperty>();
};

#endif // PATHEFFECT_H
