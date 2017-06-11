#include "patheffect.h"

PathEffect::PathEffect(const PathEffectType &type) {
    mPathEffectType = type;
}

sk_sp<SkPathEffect> PathEffect::getPathEffect() {
    return mSkPathEffect;
}

qreal PathEffect::getMargin() { return 0.; }

DiscretePathEffect::DiscretePathEffect() :
    PathEffect(DISCRETE_PATH_EFFECT) {
    prp_setName("discrete effect");
    mSegLength->prp_setName("segment length");
    mMaxDev->prp_setName("max deviation");

    connect(mSegLength.data(), SIGNAL(valueChangedSignal(qreal)),
            this, SLOT(updatePathEffect()));
    connect(mMaxDev.data(), SIGNAL(valueChangedSignal(qreal)),
            this, SLOT(updatePathEffect()));

    ca_addChildAnimator(mSegLength.data());
    ca_addChildAnimator(mMaxDev.data());
}

qreal DiscretePathEffect::getMargin() {
    return mMaxDev->qra_getCurrentValue();
}

Property *DiscretePathEffect::makeDuplicate() {
    DiscretePathEffect *newEffect = new DiscretePathEffect();
    makeDuplicate(newEffect);
    return newEffect;
}

void DiscretePathEffect::makeDuplicate(Property *target) {
    DiscretePathEffect *effectTarget = (DiscretePathEffect*)target;

    effectTarget->duplicateAnimatorsFrom(mSegLength.data(),
                                         mMaxDev.data());
}

void DiscretePathEffect::duplicateAnimatorsFrom(QrealAnimator *segLen,
                                                QrealAnimator *maxDev) {
    segLen->makeDuplicate(mSegLength.data());
    maxDev->makeDuplicate(mMaxDev.data());
}

void DiscretePathEffect::updatePathEffect() {
    mSkPathEffect = SkDiscretePathEffect::Make(
                mSegLength->qra_getCurrentValue(),
                mMaxDev->qra_getCurrentValue());
}
