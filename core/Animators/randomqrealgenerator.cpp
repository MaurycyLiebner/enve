#include "randomqrealgenerator.h"
#include "pointhelpers.h"
#include "qrealanimator.h"
#include "Animators/qrealkey.h"
#include "pointhelpers.h"
#include "Properties/comboboxproperty.h"
#include "Animators/intanimator.h"

RandomQrealGenerator::RandomQrealGenerator() :
    QrealValueEffect("noise") {
    mTime = SPtrCreate(QrealAnimator)(0, 0, 9999, 1, "time");
    ca_addChildAnimator(mTime);
    mSmoothness = QrealAnimator::create0to1Animator("smoothness");
    ca_addChildAnimator(mSmoothness);
    mMaxDev = SPtrCreate(QrealAnimator)(0, 0, 999, 1, "amplitude");
    ca_addChildAnimator(mMaxDev);
    mType = SPtrCreate(ComboBoxProperty)("type",
                QStringList() << "add" << "subtract" << "overlay");
    mType->setCurrentValue(2);
    ca_addChildAnimator(mType);

    mSeedAssist = SPtrCreate(IntAnimator)("seed");
    mSeedAssist->setValueRange(0, 9999);
    mSeedAssist->setCurrentIntValue(0);
    ca_addChildAnimator(mSeedAssist);
}

void RandomQrealGenerator::anim_setAbsFrame(const int &frame) {
    ComplexAnimator::anim_setAbsFrame(frame);
    if(mMaxDev->getCurrentEffectiveValue() < 0.001) return;
    anim_callFrameChangeUpdater();
}

FrameRange RandomQrealGenerator::prp_getIdenticalRelRange(const int &relFrame) const {
    if(mMaxDev->getEffectiveValue(relFrame) < 0.001) {
        return ComplexAnimator::prp_getIdenticalRelRange(relFrame);
    }
    return {relFrame, relFrame};
}

qreal RandomQrealGenerator::getDevAtRelFrame(const qreal &relFrame) {
    const int seed = mSeedAssist->getCurrentIntValueAtRelFrame(relFrame);
    const qreal time = mTime->getEffectiveValue(relFrame);
    const qreal smooth = mSmoothness->getEffectiveValue(relFrame);
    const qreal maxDev = mMaxDev->getEffectiveValue(relFrame);
    const int type = mType->getCurrentValue();
    const int iPrevTime = qFloor(time);
    const int iNextTime = qCeil(time);
    const qreal min = (type == 0 ? 0 : (type == 1 ? -1 : -0.5));
    const qreal max = (type == 0 ? 1 : (type == 1 ? 0 : 0.5));
    QRandomGenerator gen;
    gen.seed(static_cast<uint>(seed + iPrevTime));
    const qreal prevDev = (gen.generateDouble()*(max - min) + min)*maxDev;
    gen.seed(static_cast<uint>(seed + iNextTime));
    const qreal nextDev = (gen.generateDouble()*(max - min) + min)*maxDev;

    const qreal frac = time - iPrevTime;

    if(isZero4Dec(smooth)) return frac*nextDev + (1 - frac)*prevDev;

    qCubicSegment2D seg({0, 0}, {smooth, 0}, {1 - smooth, 1}, {1, 1});
    qreal bestT;
    seg.xSeg().minDistanceTo(frac, &bestT);
    const qreal smoothFrac = seg.ySeg().valAtT(bestT);
    return smoothFrac*nextDev + (1 - smoothFrac)*prevDev;
}
