#include "displacepatheffect.h"
#include "Animators/intanimator.h"
#include "Properties/boolproperty.h"
#include "pointhelpers.h"

DisplacePathEffect::DisplacePathEffect(const bool &outlinePathEffect) :
    PathEffect("displace effect", DISPLACE_PATH_EFFECT, outlinePathEffect) {
    mSegLength = SPtrCreate(QrealAnimator)("segment length");
    mMaxDev = SPtrCreate(QrealAnimator)("max deviation");
    mSmoothness = QrealAnimator::create0to1Animator("smoothness");
    mLengthBased = SPtrCreate(BoolPropertyContainer)("length inc");
    mSeed = SPtrCreate(QrealAnimator)(qrand() % 9999, 0, 9999, 1, "smooth seed");

    mSegLength->setValueRange(1, 1000);
    mSegLength->setCurrentBaseValue(20);

    mMaxDev->setValueRange(0, 1000);
    mMaxDev->setCurrentBaseValue(20);

    ca_addChildAnimator(mSeed);
    ca_addChildAnimator(mMaxDev);
    ca_addChildAnimator(mLengthBased);

    mLengthBased->ca_addChildAnimator(mSegLength);
    mLengthBased->ca_addChildAnimator(mSmoothness);
    mLengthBased->setValue(false);
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
    const qreal baseSeed = mSeed->getEffectiveValue(relFrame);

    const qreal qMaxDev = mMaxDev->getEffectiveValue(relFrame);
    const qreal qSegLen = mSegLength->getEffectiveValue(relFrame);
    const qreal qSmooth = mSmoothness->getEffectiveValue(relFrame);    

    dst->reset();

    const SkScalar maxDev = toSkScalar(qMaxDev);
    const SkScalar segLen = toSkScalar(qSegLen);
    const SkScalar smooth = toSkScalar(qSmooth);

    if(mLengthBased->getValue()) {
        gAtomicDisplaceFilterPath(baseSeed, dst, src, maxDev, segLen, smooth);
    } else {
        gAtomicDisplaceFilterPath(baseSeed, dst, src, maxDev);
    }
}
