#include "randomqrealgenerator.h"
#include "pointhelpers.h"
#include "qrealanimator.h"
#include "Animators/qrealkey.h"
#include "pointhelpers.h"
#include "PropertyUpdaters/randomqrealgeneratorupdater.h"
#include "Properties/comboboxproperty.h"
#include "Properties/intproperty.h"

RandomQrealGenerator::RandomQrealGenerator(const int &firstFrame,
                                           const int &lastFrame) :
    QrealValueEffect("noise") {
    mPeriod = SPtrCreate(QrealAnimator)(10., 1., 999., 1., "period");
    mPeriod->prp_setBlockedUpdater(
                SPtrCreate(RandomQrealGeneratorUpdater)(this));
    ca_addChildAnimator(mPeriod);
    mSmoothness = QrealAnimator::create0to1Animator("smoothness");
    ca_addChildAnimator(mSmoothness);
    mMaxDev = SPtrCreate(QrealAnimator)(0., 0., 999., 1., "amplitude");
    ca_addChildAnimator(mMaxDev);
    mType = SPtrCreate(ComboBoxProperty)("type",
                QStringList() << "add" << "subtract" << "overlay");
    ca_addChildAnimator(mType);

    mSeedAssist = SPtrCreate(IntProperty)("seed");
    mSeedAssist->setValueRange(0, 9999);
    mSeedAssist->setCurrentValue(0);
    ca_addChildAnimator(mSeedAssist);
    mSeedAssist->prp_setBlockedUpdater(
                SPtrCreate(RandomQrealGeneratorUpdater)(this));

    mFirstFrame = firstFrame;
    mLastFrame = lastFrame;
    generateData();
}

void RandomQrealGenerator::prp_setAbsFrame(const int &frame) {
    ComplexAnimator::prp_setAbsFrame(frame);
    if(mMaxDev->qra_getCurrentEffectiveValue() < 0.001) return;
    anim_callFrameChangeUpdater();
}

FrameRange RandomQrealGenerator::prp_getIdenticalRelFrameRange(const int &relFrame) const {
    if(mMaxDev->qra_getEffectiveValueAtRelFrame(relFrame) < 0.001) {
        return ComplexAnimator::prp_getIdenticalRelFrameRange(relFrame);
    }
    return {relFrame, relFrame};
}

qreal RandomQrealGenerator::getDevAtRelFrameF(const qreal &relFrame) {
//    if(relFrame > mLastFrame) {
//        mLastFrame = relFrame;
//        generateData();
//    }
//    if(relFrame < mFirstFrame) {
//        mFirstFrame = relFrame;
//        generateData();
//    }
    int relFrameRel = qRound(relFrame) % (mLastFrame - 1);

    qreal maxDev = mMaxDev->getCurrentEffectiveValueAtRelFrame(relFrameRel);
    int idBefore = getClosestLowerFrameId(0, mFrameValues.count() - 1, relFrameRel);
    const FrameValue &frameValueBefore = mFrameValues.at(idBefore);
    qreal frameBefore = frameValueBefore.frame;
    qreal valueBefore = frameValueBefore.value;
    qreal smoothnessBefore =
            mSmoothness->qra_getEffectiveValueAtRelFrame(frameBefore);

    int idAfter = idBefore + 1;
    const FrameValue &frameValueAfter = mFrameValues.at(idAfter);
    qreal frameAfter = frameValueAfter.frame;
    qreal valueAfter = frameValueAfter.value;

    int currentType = mType->getCurrentValue();
    if(currentType == 2) { // overlay
        valueBefore = valueBefore - .5;
        valueAfter = valueAfter - .5;
    } else if(currentType == 1) { // subtract
        valueBefore = -valueBefore;
        valueAfter = -valueAfter;
    }

    valueBefore = valueBefore*maxDev;
    valueAfter = valueAfter*maxDev;

    if(qAbs(relFrameRel - frameBefore) < 0.01) return valueBefore;
    if(qAbs(relFrameRel - frameAfter) < 0.01) return valueAfter;
    qreal smoothnessAfter =
            mSmoothness->qra_getEffectiveValueAtRelFrame(frameAfter);

    qreal t;
    if(smoothnessBefore < 0.001 && smoothnessAfter < 0.001) {
        qreal dVal = (valueAfter - valueBefore)/(frameAfter - frameBefore);
        return (relFrameRel - frameBefore)*dVal + valueBefore;
    } else {
        qreal c1F = frameBefore + smoothnessBefore*(frameAfter - frameBefore);
        qreal c2F = frameAfter - smoothnessAfter*(frameAfter - frameBefore);

        qCubicSegment1D seg(frameBefore, c1F, c2F, frameAfter);
        seg.minDistanceTo(relFrameRel, &t);
    }
    return t*valueAfter + (1 - t)*valueBefore;
}

qreal RandomQrealGenerator::getDevAtRelFrame(const int &relFrame) {
    return getDevAtRelFrameF(relFrame);
}

int RandomQrealGenerator::getClosestLowerFrameId(const int &minId,
                                                 const int &maxId,
                                                 const int &targetFrame) {
    if(maxId - minId < 2) {
        if(mFrameValues.at(maxId).frame < targetFrame) return maxId;
        return minId;
    }
    int guessId = (minId + maxId)/2;
    const FrameValue &frameValue = mFrameValues.at(guessId);
    qreal frameGuess = frameValue.frame;
    if(frameGuess > targetFrame) {
        return getClosestLowerFrameId(minId, guessId, targetFrame);
    }
    return getClosestLowerFrameId(guessId, maxId, targetFrame);
}

qreal RandomQrealGenerator::getDeltaX(const int &relFrame) {
    qreal totDeltaX = 0;
    qreal A = 0;
    qreal prevPeriod = mPeriod->qra_getEffectiveValueAtRelFrame(relFrame);
    int prevFrame = relFrame;
    int nextFrame = relFrame;
    qreal nextPeriod = prevPeriod;
    while(true) {
        QrealKey *nextKey =
                GetAsPtr(mPeriod->anim_getNextKey(nextFrame), QrealKey);
        if(nextKey == nullptr) {
            totDeltaX += qMax(0., prevPeriod - A/prevPeriod);
            break;
        } else {
            nextPeriod = nextKey->getValue();
            prevFrame = nextFrame;
            nextFrame = nextKey->getRelFrame();
            qreal maxDeltaX = nextFrame - prevFrame;
            qreal y1 = prevPeriod;
            qreal y2 = nextPeriod;
            qreal x1 = prevFrame;
            qreal x2 = nextFrame;
            qreal deltaX;
            if(qAbs(y1 - y2) < 0.01) {
                deltaX = prevPeriod - A/prevPeriod;
            } else {
                qreal c1 = x1*x1*y1 - 2*x1*x2*y1 - 2*x1*y1*y1 + 2*x1*y1*y2 +
                        x2*x2*y1 + 2*x2*y1*y1 - 2*x2*y1*y2;
                qreal c2 = qSqrt(-(x1 - x2)*(x1 - x2)*(x2 - x1 + 2*(y1 - y2))*
                                 (2*A*y2 -2*A*y1 + x1*y1*y1 - x2*y1*y1));
                qreal c3 = (y1 - y2)*(x2 - x1 + 2*(y1 - y2));
                qreal deltaX1 = (c1 - c2)/c3;
                qreal deltaX2 = (c1 + c2)/c3;
                deltaX = qMax(deltaX1, deltaX2);
            }
            deltaX = qMax(0., qMin(maxDeltaX, deltaX));
            A += deltaX*deltaX*(y2 - y1)/(x2 - x1)*0.5 + y1*deltaX;
            totDeltaX += deltaX;
            if(nextPeriod - totDeltaX < 0.01) {
                break;
            }
        }
        prevPeriod = nextPeriod;
        prevFrame = nextFrame;
    }
    return qMax(1., totDeltaX);
}

void RandomQrealGenerator::generateData() {
    qsrand(mSeedAssist->getValue());
    mFrameValues.clear();
    qreal currFrame = mFirstFrame;
    while(currFrame < mLastFrame) {
        //qreal maxDev = mMaxDev->qra_getValueAtRelFrame(currFrame);
        mFrameValues << FrameValue(currFrame, gRandF(0., 1.));
        currFrame += getDeltaX(currFrame);
    }
    prp_updateInfluenceRangeAfterChanged();
}
