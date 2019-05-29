#include "displacepatheffect.h"
#include "Animators/intanimator.h"
#include "Properties/boolproperty.h"
#include "pointhelpers.h"

DisplacePathEffect::DisplacePathEffect(const bool &outlinePathEffect) :
    PathEffect("displace effect", DISPLACE_PATH_EFFECT, outlinePathEffect) {
    mSegLength = SPtrCreate(QrealAnimator)("segment length");
    mMaxDev = SPtrCreate(QrealAnimator)("max deviation");
    mSmoothness = QrealAnimator::create0to1Animator("smoothness");
    mRandomize = SPtrCreate(BoolPropertyContainer)("randomize");
    mLengthBased = SPtrCreate(BoolPropertyContainer)("length inc");
    mRandomizeStep = SPtrCreate(IntAnimator)("randomize step");
    mSmoothTransform = SPtrCreate(BoolPropertyContainer)("smooth progression");
    mEasing = QrealAnimator::create0to1Animator("ease in/out");
    mSeed = IntAnimator::sCreateSeed();
    mRepeat = SPtrCreate(BoolProperty)("repeat");

    mSegLength->setValueRange(1, 1000);
    mSegLength->setCurrentBaseValue(20);

    mMaxDev->setValueRange(0, 1000);
    mMaxDev->setCurrentBaseValue(20);

    mRandomizeStep->setIntValueRange(1, 99);

    mRepeat->setValue(false);

    ca_addChildAnimator(mSeed);
    ca_addChildAnimator(mMaxDev);
    ca_addChildAnimator(mRandomize);
    ca_addChildAnimator(mLengthBased);

    mLengthBased->ca_addChildAnimator(mSegLength);
    mLengthBased->ca_addChildAnimator(mSmoothness);
    mLengthBased->setValue(false);

    mRandomize->ca_addChildAnimator(mRandomizeStep);
    mRandomize->ca_addChildAnimator(mSmoothTransform);
    mSmoothTransform->ca_addChildAnimator(mEasing);
    mSmoothTransform->setValue(false);
    mRandomize->ca_addChildAnimator(mRepeat);
    mRandomize->setValue(false);
}

std::pair<QPointF, QPointF> posAndTanToC1C2(const PosAndTan& prev,
                                            const PosAndTan& next,
                                            const qreal& smooth) {
    const qreal thirdDist = 0.333*pointToLen(next.fPos - prev.fPos);
    const auto c1Vector = scalePointToNewLen(prev.fTan, thirdDist);
    const QPointF c1 = prev.fPos + c1Vector*smooth;
    const auto c2Vector = scalePointToNewLen(next.fTan, thirdDist);
    const QPointF c2 = next.fPos - c2Vector*smooth;
    return {c1, c2};
}

void perterb(PosAndTan& posAndTan, const qreal& dev) {
    posAndTan.fTan = scalePointToNewLen(posAndTan.fTan, dev);
    const auto displ = gRotPt(posAndTan.fTan, 90);
    posAndTan.fPos += displ;
}

void DisplacePathEffect::apply(const qreal &relFrame,
                               const SkPath &src,
                               SkPath * const dst) {
    mSeedAssist = static_cast<uint>(mSeed->getCurrentIntValue());
    const int randStep = mRandomizeStep->getBaseIntValue(relFrame);
    uint32_t nextSeed;
    if(mRepeat->getValue()) {
        if((qFloor(relFrame / randStep)) % 2 == 1) {
            nextSeed = mSeedAssist;
            mSeedAssist++;
        } else {
            nextSeed = mSeedAssist + 1;
        }
    } else if(mRandomize->getValue()) {
        mSeedAssist += static_cast<uint>(qAbs(qFloor(relFrame/randStep)));
        nextSeed = mSeedAssist - 1;
    }

    const qreal qMaxDev = mMaxDev->getEffectiveValue(relFrame);
    const qreal qSegLen = mSegLength->getEffectiveValue(relFrame);
    const qreal qSmooth = mSmoothness->getEffectiveValue(relFrame);    

    dst->reset();

    const SkScalar maxDev = toSkScalar(qMaxDev);
    const SkScalar segLen = toSkScalar(qSegLen);
    const SkScalar smooth = toSkScalar(qSmooth);

    if(mSmoothTransform->getValue() && mRandomize->getValue()) {
        SkPath path1;
        if(mLengthBased->getValue()) {
            gAtomicDisplaceFilterPath(&path1, src, maxDev, segLen, smooth, mSeedAssist);
        } else {
            gAtomicDisplaceFilterPath(&path1, src, maxDev, mSeedAssist);
        }
        SkPath path2;
        if(mLengthBased->getValue()) {
            gAtomicDisplaceFilterPath(&path2, src, maxDev, segLen, smooth, nextSeed);
        } else {
            gAtomicDisplaceFilterPath(&path2, src, maxDev, nextSeed);
        }
        qreal weight = qAbs(qFloor(relFrame) % randStep)*1./randStep;
        const qreal easing = mEasing->getEffectiveValue(relFrame);
        if(easing > 0.0001) {
            qCubicSegment1D seg(0, easing, 1 - easing, 1);
            qreal tT;
            seg.minDistanceTo(weight, &tT);
            weight = gCubicValueAtT({0, 0, 1, 1}, tT);
        }
        path1.interpolate(path2, toSkScalar(weight), dst);
    } else {
        if(mLengthBased->getValue()) {
            gAtomicDisplaceFilterPath(dst, src, maxDev, segLen, smooth, mSeedAssist);
        } else {
            gAtomicDisplaceFilterPath(dst, src, maxDev, mSeedAssist);
        }
    }
}

FrameRange DisplacePathEffect::prp_getIdenticalRelRange(
        const int &relFrame) const {
    if(mRandomize->getValue()) {
        if(mSmoothTransform->getValue()) {
            return {relFrame, relFrame};
        } else {
            const int frameStep = mRandomizeStep->getBaseIntValue(relFrame);
            const int min = relFrame - relFrame % frameStep;
            const auto otherRange = PathEffect::prp_getIdenticalRelRange(relFrame);
            const FrameRange stepRange{min, min + frameStep - 1};
            return stepRange*otherRange;
        }
    }
    return PathEffect::prp_getIdenticalRelRange(relFrame);
}
