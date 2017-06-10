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
    PathEffect(const PathEffectType &type) {
        mPathEffectType = type;
    }

    virtual void updatePathEffect() = 0;
    sk_sp<SkPathEffect> getPathEffect() {
        return mSkPathEffect;
    }

protected:
    PathEffectType mPathEffectType;
    sk_sp<SkPathEffect> mSkPathEffect;
};

class DiscretePathEffect : public PathEffect {
    Q_OBJECT
public:
    DiscretePathEffect() : PathEffect(DISCRETE_PATH_EFFECT) {
        mSegLength->prp_setName("segment length");
        mMaxDev->prp_setName("max deviation");

        connect(mSegLength.data(), SIGNAL(valueChangedSignal(qreal)),
                this, SLOT(updatePathEffect()));
        connect(mMaxDev.data(), SIGNAL(valueChangedSignal(qreal)),
                this, SLOT(updatePathEffect()));

        ca_addChildAnimator(mSegLength.data());
        ca_addChildAnimator(mMaxDev.data());
    }
protected slots:
    void updatePathEffect() {
        mSkPathEffect = SkDiscretePathEffect::Make(
                    mSegLength->qra_getCurrentValue(),
                    mMaxDev->qra_getCurrentValue());
    }
private:
    QSharedPointer<QrealAnimator> mSegLength =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mMaxDev =
            (new QrealAnimator())->ref<QrealAnimator>();
};

#endif // PATHEFFECT_H
