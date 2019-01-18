#include "displacepatheffect.h"
#include "pointhelpers.h"

DisplacePathEffect::DisplacePathEffect(const bool &outlinePathEffect) :
    PathEffect("displace effect", DISPLACE_PATH_EFFECT, outlinePathEffect) {
    mSegLength = SPtrCreate(QrealAnimator)("segment length");
    mMaxDev = SPtrCreate(QrealAnimator)("max deviation");
    mSmoothness = QrealAnimator::create0to1Animator("smoothness");
    mRandomize = SPtrCreate(BoolPropertyContainer)("randomize");
    mRandomizeStep = SPtrCreate(IntAnimator)("randomize step");
    mSmoothTransform = SPtrCreate(BoolPropertyContainer)("smooth progression");
    mEasing = QrealAnimator::create0to1Animator("ease in/out");
    mSeed = SPtrCreate(IntAnimator)("seed");
    mRepeat = SPtrCreate(BoolProperty)("repeat");

    mSeed->setIntValueRange(0, 9999);
    mSeed->setCurrentIntValue(qrand() % 9999);

    mSegLength->qra_setValueRange(1., 1000.);
    mSegLength->qra_setCurrentValue(20.);

    mMaxDev->qra_setValueRange(0., 1000.);
    mMaxDev->qra_setCurrentValue(20.);

    mRandomizeStep->setIntValueRange(1, 99);

    mRepeat->setValue(false);

    ca_addChildAnimator(mSeed);
    ca_addChildAnimator(mSegLength);
    ca_addChildAnimator(mMaxDev);
    ca_addChildAnimator(mSmoothness);
    ca_addChildAnimator(mRandomize);

    mRandomize->ca_addChildAnimator(mRandomizeStep);
    mRandomize->ca_addChildAnimator(mSmoothTransform);
    mSmoothTransform->ca_addChildAnimator(mEasing);
    mSmoothTransform->setValue(false);
    mRandomize->ca_addChildAnimator(mRepeat);
    mRandomize->setValue(false);
}

void DisplacePathEffect::filterPathForRelFrame(const int &relFrame,
                                               const SkPath &src,
                                               SkPath *dst,
                                               const qreal &scale,
                                               const bool &) {
    dst->reset();
    qsrand(static_cast<uint>(mSeed->getCurrentIntValue()));
    mSeedAssist = qrand() % 999999;
    int randStep = mRandomizeStep->getCurrentIntValueAtRelFrame(relFrame);
    uint32_t nextSeed;
    if(mRepeat->getValue()) {
        if(qAbs(relFrame / randStep) % 2 == 1) {
            nextSeed = mSeedAssist;
            mSeedAssist++;
        } else {
            nextSeed = mSeedAssist + 1;
        }
    } else if(mRandomize->getValue()) {
        mSeedAssist += qAbs(relFrame / randStep);
        nextSeed = mSeedAssist - 1;
    }
    if(mSmoothTransform->getValue() && mRandomize->getValue()) {
        SkPath path1;
        gDisplaceFilterPath(&path1, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrame(relFrame)/scale,
                           mSegLength->qra_getEffectiveValueAtRelFrame(relFrame)/scale,
                           mSmoothness->qra_getEffectiveValueAtRelFrame(relFrame),
                           mSeedAssist);
        SkPath path2;
        qsrand(mSeed->getCurrentIntValue());
        gDisplaceFilterPath(&path2, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrame(relFrame + randStep)/scale,
                           mSegLength->qra_getEffectiveValueAtRelFrame(relFrame + randStep)/scale,
                           mSmoothness->qra_getEffectiveValueAtRelFrame(relFrame + randStep),
                           nextSeed);
        qreal weight = qAbs(relFrame % randStep)*1./randStep;
        qreal easing = mEasing->getCurrentEffectiveValueAtRelFrame(relFrame);
        if(easing > 0.0001) {
            qCubicSegment1D seg(0, easing, 1 - easing, 1);
            qreal tT;
            seg.minDistanceTo(weight, &tT);
            weight = gCubicValueAtT({0., 0., 1., 1.}, tT);
        }
        path1.interpolate(path2, weight, dst);
    } else {
        gDisplaceFilterPath(dst, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrame(relFrame)/scale,
                           mSegLength->qra_getEffectiveValueAtRelFrame(relFrame)/scale,
                           mSmoothness->qra_getEffectiveValueAtRelFrame(relFrame),
                           mSeedAssist);
    }
}

void DisplacePathEffect::filterPathForRelFrameF(const qreal &relFrame,
                                                const SkPath &src,
                                                SkPath *dst, const bool &) {
    dst->reset();
    qsrand(mSeed->getCurrentIntValue());
    mSeedAssist = qrand() % 999999;
    int randStep = mRandomizeStep->getCurrentIntValueAtRelFrameF(relFrame);
    uint32_t nextSeed;
    if(mRepeat->getValue()) {
        if((qFloor(relFrame / randStep)) % 2 == 1) {
            nextSeed = mSeedAssist;
            mSeedAssist++;
        } else {
            nextSeed = mSeedAssist + 1;
        }
    } else if(mRandomize->getValue()) {
        mSeedAssist += qFloor(relFrame / randStep);
        nextSeed = mSeedAssist - 1;
    }
    if(mSmoothTransform->getValue() && mRandomize->getValue()) {
        SkPath path1;
        gDisplaceFilterPath(&path1, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrameF(relFrame),
                           mSegLength->qra_getEffectiveValueAtRelFrameF(relFrame),
                           mSmoothness->qra_getEffectiveValueAtRelFrameF(relFrame),
                           mSeedAssist);
        SkPath path2;
        qsrand(mSeed->getCurrentIntValue());
        gDisplaceFilterPath(&path2, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrameF(relFrame + randStep),
                           mSegLength->qra_getEffectiveValueAtRelFrameF(relFrame + randStep),
                           mSmoothness->qra_getEffectiveValueAtRelFrameF(relFrame + randStep),
                           nextSeed);
        qreal weight = qAbs(qFloor(relFrame) % randStep)*1./randStep;
        qreal easing = mEasing->getCurrentEffectiveValueAtRelFrameF(relFrame);
        if(easing > 0.0001) {
            qCubicSegment1D seg(0, easing, 1 - easing, 1);
            qreal tT;
            seg.minDistanceTo(weight, &tT);
            weight = gCubicValueAtT({0., 0., 1., 1.}, tT);
        }
        path1.interpolate(path2, weight, dst);
    } else {
        gDisplaceFilterPath(dst, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrameF(relFrame),
                           mSegLength->qra_getEffectiveValueAtRelFrameF(relFrame),
                           mSmoothness->qra_getEffectiveValueAtRelFrameF(relFrame),
                           mSeedAssist);
    }
}
