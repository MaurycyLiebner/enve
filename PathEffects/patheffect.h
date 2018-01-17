#ifndef PATHEFFECT_H
#define PATHEFFECT_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimator.h"
#include "Animators/qpointfanimator.h"
#include "Animators/intanimator.h"
#include "Properties/boolproperty.h"
#include "Properties/comboboxproperty.h"
#include "Animators/intanimator.h"
#include "skiaincludes.h"
#include "Properties/boxtargetproperty.h"
class PathBox;
class PathEffectAnimators;
enum PathEffectType : short {
    DISPLACE_PATH_EFFECT,
    DASH_PATH_EFFECT,
    DUPLICATE_PATH_EFFECT,
    SOLIDIFY_PATH_EFFECT,
    SUM_PATH_EFFECT,
    GROUP_SUM_PATH_EFFECT
};
class PathEffect;
typedef QSharedPointer<ComboBoxProperty> ComboBoxPropertyQSPtr;
typedef QSharedPointer<BoxTargetProperty> BoxTargetPropertyQSPtr;

class PathEffectMimeData : public QMimeData {
    Q_OBJECT
public:
    PathEffectMimeData(PathEffect *target) : QMimeData() {
        mPathEffect = target;
    }

    PathEffect *getPathEffect() {
        return mPathEffect;
    }

    bool hasFormat(const QString &mimetype) const {
        if(mimetype == "patheffect") return true;
        return false;
    }

private:
    PathEffect *mPathEffect;
};

class PathEffect : public ComplexAnimator {
    Q_OBJECT
public:
    PathEffect(const PathEffectType &type,
               const bool &outlinePathEffect);

    const PathEffectType &getEffectType() {
        return mPathEffectType;
    }

    virtual void filterPathForRelFrame(const int &,
                                       const SkPath &,
                                       SkPath *,
                                       const bool &) = 0;
    virtual void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);

    bool applyBeforeThickness() {
        if(mApplyBeforeThickness == NULL) return false;
        return mApplyBeforeThickness->getValue();
    }
    void setParentEffectAnimators(PathEffectAnimators *parent) {
        mParentEffectAnimators = parent;
    }

    PathEffectAnimators *getParentEffectAnimators() {
        return mParentEffectAnimators;
    }

    QMimeData *SWT_createMimeData() {
        return new PathEffectMimeData(this);
    }

    bool SWT_isPathEffect() { return true; }

    void setIsOutlineEffect(const bool &bT) {
        if(bT == mOutlineEffect) return;
        mOutlineEffect = bT;
        if(mOutlineEffect) {
            mApplyBeforeThickness = (new BoolProperty())->ref<BoolProperty>();
            mApplyBeforeThickness->prp_setName("pre-thickness");
            ca_addChildAnimator(mApplyBeforeThickness.data());
        } else if(mApplyBeforeThickness != NULL) {
            ca_removeChildAnimator(mApplyBeforeThickness.data());
            mApplyBeforeThickness.reset();
        }
    }

    void switchVisible() {
        setVisible(!mVisible);
    }

    void setVisible(const bool &bT) {
        if(bT == mVisible) return;
        mVisible = bT;
        prp_updateInfluenceRangeAfterChanged();
    }

    const bool &isVisible() {
        return mVisible;
    }

    virtual bool hasReasonsNotToApplyUglyTransform() { return false; }
protected:
    bool mVisible = true;
    bool mOutlineEffect = false;
    BoolPropertyQSPtr mApplyBeforeThickness;
    PathEffectType mPathEffectType;
    PathEffectAnimators *mParentEffectAnimators = NULL;
};

class DisplacePathEffect : public PathEffect {
    Q_OBJECT
public:
    DisplacePathEffect(const bool &outlinePathEffect);

    void filterPathForRelFrame(const int &relFrame,
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
    QSharedPointer<QrealAnimator> mSegLength =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mMaxDev =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mSmoothness =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<BoolPropertyContainer> mRandomize =
            (new BoolPropertyContainer())->ref<BoolPropertyContainer>();
    QSharedPointer<IntAnimator> mRandomizeStep =
            (new IntAnimator())->ref<IntAnimator>();
    QSharedPointer<BoolPropertyContainer> mSmoothTransform =
            (new BoolPropertyContainer())->ref<BoolPropertyContainer>();
    QSharedPointer<QrealAnimator> mEasing =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<IntAnimator> mSeed =
            (new IntAnimator())->ref<IntAnimator>();
    QSharedPointer<BoolProperty> mRepeat =
            (new BoolProperty())->ref<BoolProperty>();
    uint32_t mSeedAssist = 0;
};

class DuplicatePathEffect : public PathEffect {
    Q_OBJECT
public:
    DuplicatePathEffect(const bool &outlinePathEffect);

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src,
                               SkPath *dst,
                               const bool &);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    QPointFAnimatorQSPtr mTranslation;
};

class SolidifyPathEffect : public PathEffect {
    Q_OBJECT
public:
    SolidifyPathEffect(const bool &outlinePathEffect);

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src,
                               SkPath *dst,
                               const bool &);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    QrealAnimatorQSPtr mDisplacement;
};

class SumPathEffect : public PathEffect {
    Q_OBJECT
public:
    SumPathEffect(PathBox *parentPath,
                  const bool &outlinePathEffect);


    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src,
                               SkPath *dst,
                               const bool &);

    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);

    bool hasReasonsNotToApplyUglyTransform() {
        return true;//mBoxTarget->getTarget() != NULL;
    }
private:
    PathBox *mParentPathBox;
    ComboBoxPropertyQSPtr mOperationType;
    BoxTargetPropertyQSPtr mBoxTarget;
};

class GroupLastPathSumPathEffect : public PathEffect {
    Q_OBJECT
public:
    GroupLastPathSumPathEffect(BoxesGroup *parentPath,
                               const bool &outlinePathEffect);


    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src,
                               SkPath *dst,
                               const bool &groupPathSum);

    void writeProperty(QIODevice *target) {
        Q_UNUSED(target);
    }
    void readProperty(QIODevice *target) {
        Q_UNUSED(target);
    }

//    bool hasReasonsNotToApplyUglyTransform() {
//        return true;//mBoxTarget->getTarget() != NULL;
//    }

    void setParentGroup(BoxesGroup *parent);
    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);
private:
    BoxesGroup *mParentGroup;
};

#endif // PATHEFFECT_H
