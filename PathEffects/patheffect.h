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

    virtual void updatePathEffect() = 0;
    sk_sp<SkPathEffect> getPathEffect();

    virtual qreal getMargin();

    virtual Property *makeDuplicate() = 0;
    virtual void makeDuplicate(Property *target) = 0;
protected:
    PathEffectType mPathEffectType;
    sk_sp<SkPathEffect> mSkPathEffect;
};

class DiscretePathEffect : public PathEffect {
    Q_OBJECT
public:
    DiscretePathEffect();

    qreal getMargin();

    Property *makeDuplicate();

    void makeDuplicate(Property *target);

    void duplicateAnimatorsFrom(QrealAnimator *segLen,
                                QrealAnimator *maxDev);

protected slots:
    void updatePathEffect();
private:
    QSharedPointer<QrealAnimator> mSegLength =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mMaxDev =
            (new QrealAnimator())->ref<QrealAnimator>();
};

#endif // PATHEFFECT_H
