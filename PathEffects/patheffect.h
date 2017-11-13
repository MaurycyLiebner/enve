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
class PathEffectAnimators;
enum PathEffectType : short {
    DISPLACE_PATH_EFFECT,
    DASH_PATH_EFFECT,
    DUPLICATE_PATH_EFFECT,
    SOLIDIFY_PATH_EFFECT,
    SUM_PATH_EFFECT
};
class PathEffect;

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

    virtual Property *makeDuplicate() = 0;
    virtual void makeDuplicate(Property *target) = 0;
    virtual void filterPath(const SkPath &, SkPath *) = 0;
    virtual void filterPathForRelFrame(const int &,
                                       const SkPath &,
                                       SkPath *) = 0;
    virtual void writeProperty(QIODevice *target);
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

    void setVisible(const bool &bT) {
        mVisible = bT;
    }

    const bool &isVisible() {
        return mVisible;
    }

    virtual bool hasReasonsNotToApplyUglyTransform() { return false; }
protected:
    bool mVisible = true;
    bool mOutlineEffect = false;
    QSharedPointer<BoolProperty> mApplyBeforeThickness;
    PathEffectType mPathEffectType;
    PathEffectAnimators *mParentEffectAnimators = NULL;
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
    QSharedPointer<BoolProperty> mRandomize =
            (new BoolProperty())->ref<BoolProperty>();
    QSharedPointer<IntAnimator> mRandomizeStep =
            (new IntAnimator())->ref<IntAnimator>();
    QSharedPointer<BoolProperty> mSmoothTransform =
            (new BoolProperty())->ref<BoolProperty>();
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
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    QSharedPointer<QPointFAnimator> mTranslation =
            (new QPointFAnimator())->ref<QPointFAnimator>();
};

class SolidifyPathEffect : public PathEffect {
    Q_OBJECT
public:
    SolidifyPathEffect(const bool &outlinePathEffect);

    Property *makeDuplicate();

    void makeDuplicate(Property *target);

    void duplicateAnimatorsFrom(QrealAnimator *trans);

    void filterPath(const SkPath &src, SkPath *dst);

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src, SkPath *dst);
    void writeProperty(QIODevice *target) {}
    void readProperty(QIODevice *target) {}
private:
    QSharedPointer<QrealAnimator> mDisplacement =
            (new QrealAnimator())->ref<QrealAnimator>();
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

    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);

    bool hasReasonsNotToApplyUglyTransform() {
        return true;//mBoxTarget->getTarget() != NULL;
    }
private:
    PathBox *mParentPathBox;
    QSharedPointer<ComboBoxProperty> mOperationType =
            (new ComboBoxProperty(
                 QStringList() << "Union" <<
                 "Difference" << "Intersection" <<
                 "Exclusion"))->ref<ComboBoxProperty>();
    QSharedPointer<BoxTargetProperty> mBoxTarget =
            (new BoxTargetProperty())->ref<BoxTargetProperty>();
};

#endif // PATHEFFECT_H
