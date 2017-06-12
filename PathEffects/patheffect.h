#ifndef PATHEFFECT_H
#define PATHEFFECT_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimator.h"
#include "skiaincludes.h"

enum PathEffectType : short {
    DISCRETE_PATH_EFFECT,
    DASH_PATH_EFFECT
};

class PathEffect : public ComplexAnimator {
    Q_OBJECT
public:
    PathEffect(const PathEffectType &type);

    virtual qreal getMargin();

    virtual Property *makeDuplicate() = 0;
    virtual void makeDuplicate(Property *target) = 0;
    virtual void filterPath(const SkPath &, SkPath *) = 0;
protected:
    PathEffectType mPathEffectType;
};

class DisplacePathEffect : public PathEffect {
    Q_OBJECT
public:
    DisplacePathEffect();

    qreal getMargin();

    Property *makeDuplicate();

    void makeDuplicate(Property *target);

    void duplicateAnimatorsFrom(QrealAnimator *segLen,
                                QrealAnimator *maxDev);

    void filterPath(const SkPath &src, SkPath *dst);
private:
    QSharedPointer<QrealAnimator> mSegLength =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mMaxDev =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mSmoothness =
            (new QrealAnimator())->ref<QrealAnimator>();
    uint32_t mSeedAssist = 0;
};

#endif // PATHEFFECT_H
