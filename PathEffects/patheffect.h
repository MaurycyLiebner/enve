#ifndef PATHEFFECT_H
#define PATHEFFECT_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimator.h"
#include "Animators/qpointfanimator.h"
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
    PathEffect(const PathEffectType &type);

    virtual Property *makeDuplicate() = 0;
    virtual void makeDuplicate(Property *target) = 0;
    virtual void filterPath(const SkPath &, SkPath *) = 0;
    virtual void filterPathForRelFrame(const int &,
                                       const SkPath &,
                                       SkPath *) = 0;
    virtual void writePathEffect(std::fstream *file);
protected:
    PathEffectType mPathEffectType;
};

class DisplacePathEffect : public PathEffect {
    Q_OBJECT
public:
    DisplacePathEffect();

    Property *makeDuplicate();

    void makeDuplicate(Property *target);

    void duplicateAnimatorsFrom(QrealAnimator *segLen,
                                QrealAnimator *maxDev);

    void filterPath(const SkPath &src, SkPath *dst);
    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src, SkPath *dst);
    void writePathEffect(std::fstream *file);
    void readDisplacePathEffect(std::fstream *file);
private:
    QSharedPointer<QrealAnimator> mSegLength =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mMaxDev =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mSmoothness =
            (new QrealAnimator())->ref<QrealAnimator>();
    uint32_t mSeedAssist = 0;
};

class DuplicatePathEffect : public PathEffect {
    Q_OBJECT
public:
    DuplicatePathEffect();

    Property *makeDuplicate();

    void makeDuplicate(Property *target);

    void duplicateAnimatorsFrom(QPointFAnimator *trans);

    void filterPath(const SkPath &src, SkPath *dst);

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src, SkPath *dst);
    void writePathEffect(std::fstream *file);
    void readDuplicatePathEffect(std::fstream *file);
private:
    QSharedPointer<QPointFAnimator> mTranslation =
            (new QPointFAnimator())->ref<QPointFAnimator>();
};

class PathBox;
#include "Properties/boxtargetproperty.h"
class SumPathEffect : public PathEffect {
    Q_OBJECT
public:
    SumPathEffect(PathBox *parentPath);

    Property *makeDuplicate() {}

    void makeDuplicate(Property *target) {}

    void filterPath(const SkPath &src, SkPath *dst) {}

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src, SkPath *dst);

    void writePathEffect(std::fstream *file);
    void readSumPathEffect(std::fstream *file);
private:
    PathBox *mParentPathBox;
    QSharedPointer<BoxTargetProperty> mBoxTarget =
            (new BoxTargetProperty())->ref<BoxTargetProperty>();
};

#endif // PATHEFFECT_H
