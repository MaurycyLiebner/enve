#include "qrealsnapshot.h"
#include "qrealkey.h"

void QrealSnapshot::appendKey(const QrealKey * const key) {
    mKeys.append({key->getStartFrame()*mFrameMultiplier,
                  key->getStartValue()*mValueMultiplier,
                  key->getRelFrame()*mFrameMultiplier,
                  key->getValue()*mValueMultiplier,
                  key->getEndFrame()*mFrameMultiplier,
                  key->getEndValue()*mValueMultiplier});
}

qreal QrealSnapshot::getValue(const qreal &relFrame) const {
    const KeySnaphot * prevKey = nullptr;
    const KeySnaphot * nextKey = nullptr;

    getPrevAndNextKey(relFrame, prevKey, nextKey);

    if(prevKey && nextKey) {
        const qCubicSegment2D seg{{qreal(prevKey->fFrame), prevKey->fValue},
                                  {prevKey->fC1Frame, prevKey->fC1Value},
                                  {nextKey->fC0Frame, nextKey->fC0Value},
                                  {qreal(nextKey->fFrame), nextKey->fValue}};
        auto frameSeg =  seg.xSeg();
        const qreal t = frameSeg.tAtLength(relFrame - frameSeg.p0());
        const auto valSeg = seg.ySeg();
        return valSeg.valAtT(t);
    } else if(prevKey) {
        return prevKey->fValue;
    } else if(nextKey) {
        return nextKey->fValue;
    }
    return mCurrentValue;
}

void QrealSnapshot::getPrevAndNextKey(const qreal &relFrame,
                                      const QrealSnapshot::KeySnaphot *&prevKey,
                                      const QrealSnapshot::KeySnaphot *&nextKey) const {
    getPrevAndNextKey(relFrame, prevKey, nextKey, 0, mKeys.count() - 1);
}

void QrealSnapshot::getPrevAndNextKey(const qreal &relFrame,
                                      const QrealSnapshot::KeySnaphot *&prevKey,
                                      const QrealSnapshot::KeySnaphot *&nextKey,
                                      const int &minId, const int &maxId) const {
    int prevId;
    int nextId;
    getPrevAndNextKeyId(relFrame, prevId, nextId, minId, maxId);
    prevKey = prevId == -1 ? nullptr : &mKeys.at(prevId);
    nextKey = nextId == -1 ? nullptr : &mKeys.at(nextId);
}

void QrealSnapshot::getPrevAndNextKeyId(const qreal &relFrame,
                                        int &prevKey, int &nextKey) const {
    getPrevAndNextKeyId(relFrame, prevKey, nextKey, 0, mKeys.count() - 1);
}

void QrealSnapshot::getPrevAndNextKeyId(const qreal &relFrame,
                                        int &prevKey, int &nextKey,
                                        const int &minId, const int &maxId) const {
    if(maxId < minId) return;
    if(maxId - minId == 0) {
        prevKey = minId;
        return;
    } else if(maxId - minId == 1) {
        const auto minKey = &mKeys[minId];
        const auto maxKey = &mKeys[maxId];
        prevKey = maxKey->fFrame > relFrame ? minId : -1;
        nextKey = minKey->fFrame < relFrame ? maxId : -1;
        return;
    }
    const int guessId = (minId + maxId)/2;
    const auto& guessKey = &mKeys[guessId];
    if(guessKey->fFrame > relFrame) {
        getPrevAndNextKeyId(relFrame, prevKey, nextKey, minId, guessId);
        return;
    } else if(guessKey->fFrame < relFrame) {
        getPrevAndNextKeyId(relFrame, prevKey, nextKey, guessId, maxId);
        return;
    }
    prevKey = guessId;
    return;
}

QrealSnapshot::Iterator::Iterator(const qreal &startFrame,
                                  const qreal &sampleStep,
                                  const QrealSnapshot * const snap) :
    mSampleFrameStep(sampleStep),
    mInvFrameSpan(1/sampleStep),
    mSnapshot(snap) {
    mNextFrame = startFrame;
    mNextValue = mSnapshot->getValue(mNextFrame);
    mCurrentFrame = startFrame;
    updateSamples();
}

qreal QrealSnapshot::Iterator::getValueAndProgress(const qreal &progress) {
    if(mStaticValue) return mPrevValue;
    qreal result;
    if(mInterpolate) {
        const qreal fracNext = (mCurrentFrame - mPrevFrame)*mInvFrameSpan;
        result = (mNextValue - mPrevValue)*fracNext + mPrevValue;
    } else result = mPrevValue;
    mCurrentFrame += progress;
    if(mCurrentFrame > mNextFrame ||
            mCurrentFrame < mPrevFrame) updateSamples();
    return result;
}

bool QrealSnapshot::Iterator::staticValue() const {
    return mStaticValue;
}

void QrealSnapshot::Iterator::updateSamples() {
    if(mSnapshot->mKeys.count() < 2) {
        mPrevFrame = -TEN_MIL;
        mNextFrame = TEN_MIL;
        mPrevValue = mSnapshot->getValue(mCurrentFrame);
        mNextValue = mPrevValue;
        mStaticValue = true;
    } else {
        mPrevFrame = mNextFrame;
        mNextFrame += mSampleFrameStep;
        mPrevValue = mNextValue;
        mNextValue = mSnapshot->getValue(mNextFrame);
        mInterpolate = !isZero4Dec(mNextValue - mPrevValue);
        mStaticValue = false;
    }
}
