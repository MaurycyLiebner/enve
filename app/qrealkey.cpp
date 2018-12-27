#include "qrealkey.h"
#include "Animators/qrealanimator.h"
#include "Animators/complexanimator.h"
#include "clipboardcontainer.h"
#include "Animators/qrealpoint.h"

QrealKey::QrealKey(const int &frame,
                   const qreal &val,
                   QrealAnimator* parentAnimator) :
    Key(parentAnimator) {
    mValue = val;
    mRelFrame = frame;
    mEndFrame = mRelFrame + 5;
    mStartFrame = mRelFrame - 5;
    mStartValue = mValue;
    mEndValue = mValue;
}

QrealKey::QrealKey(QrealAnimator *parentAnimator) :
    QrealKey(0, 0., parentAnimator) { }

stdsptr<QrealKey> QrealKey::makeQrealKeyDuplicate(
        QrealAnimator* targetParent) {
    stdsptr<QrealKey> target = SPtrCreate(QrealKey)(targetParent);
    target->setValue(mValue);
    target->setRelFrame(mRelFrame);
    target->setCtrlsMode(mCtrlsMode);
    target->setStartEnabledForGraph(mStartEnabled);
    target->setStartFrameVar(mStartFrame);
    target->setStartValueVar(mStartValue);
    target->setEndEnabledForGraph(mEndEnabled);
    target->setEndFrameVar(mEndFrame);
    target->setEndValueVar(mEndValue);
    //targetParent->appendKey(target);
    return target;
}

void QrealKey::incValue(const qreal &incBy,
                        const bool &saveUndoRedo,
                        const bool &finish,
                        const bool &callUpdater) {
    setValue(mValue + incBy, saveUndoRedo,
             finish, callUpdater);
}

bool QrealKey::getEndEnabledForGraph() const {
    return mEndEnabled;
}

bool QrealKey::getStartEnabledForGraph() const {
    return mStartEnabled;
}

QrealAnimator *QrealKey::getParentQrealAnimator() const {
    return static_cast<QrealAnimator*>(mParentAnimator.data());
}

qreal QrealKey::getPrevKeyValue() const {
    if(mParentAnimator == nullptr) return mValue;
    return getParentQrealAnimator()->qra_getPrevKeyValue(this);
}

qreal QrealKey::getNextKeyValue() const {
    if(mParentAnimator == nullptr) return mValue;
    return getParentQrealAnimator()->qra_getNextKeyValue(this);
}

//bool QrealKey::isNear(qreal frameT, qreal valueT,
//                        qreal pixelsPerFrame, qreal pixelsPerValue) {
//    qreal value = getValue();
//    qreal frame = getFrame();
//    if(qAbs(frameT - frame)*pixelsPerFrame > mRadius) return false;
//    if(qAbs(valueT - value)*pixelsPerValue > mRadius) return false;
//    return true;
//}


qreal QrealKey::getValue() const { return mValue; }

void QrealKey::setValue(qreal value,
                        const bool &saveUndoRedo,
                        const bool &finish,
                        const bool &callUpdater) {
    if(mParentAnimator != nullptr) {
        value = clamp(value,
                      getParentQrealAnimator()->getMinPossibleValue(),
                      getParentQrealAnimator()->getMaxPossibleValue());
    }
    qreal dVal = value - mValue;
    setEndValueVar(mEndValue + dVal);
    setStartValueVar(mStartValue + dVal);
    if(saveUndoRedo) {
        if(mParentAnimator != nullptr) {
//            mParentAnimator->addUndoRedo(
//                    new ChangeQrealKeyValueUndoRedo(mValue, value, this) );
        }
    }
    mValue = value;
    if(finish && callUpdater) {
        mParentAnimator->anim_updateAfterChangedKey(this);
    }
}

void QrealKey::finishValueTransform() {
    if(mParentAnimator != nullptr) {
//        mParentAnimator->addUndoRedo(
//                    new ChangeQrealKeyValueUndoRedo(mSavedValue,
//                                                    mValue, this) );
        mParentAnimator->anim_updateAfterChangedKey(this);
    }
}

void QrealKey::startValueTransform() {
    mSavedValue = mValue;
}

void QrealKey::setStartValueVar(const qreal &value) {
    mStartValue = value;
}

void QrealKey::setEndValueVar(const qreal &value) {
    mEndValue = value;
}

void QrealKey::setStartFrameVar(const qreal &startFrame) {
    mStartFrame = startFrame;
}

void QrealKey::setEndFrameVar(const qreal &endFrame) {
    mEndFrame = endFrame;
}

void QrealKey::setEndValueFrameForGraph(const qreal &endFrame) {
    setEndFrameVar(endFrame);
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void QrealKey::setStartValueFrameForGraph(const qreal &startFrame) {
    setStartFrameVar(startFrame);
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void QrealKey::setStartValueForGraph(const qreal &value) {
    setStartValueVar(value);
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void QrealKey::setEndValueForGraph(const qreal &value) {
    setEndValueVar(value);
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void QrealKey::scaleFrameAndUpdateParentAnimator(
        const int &relativeToFrame, const qreal &scaleFactor,
        const bool& useSavedFrame) {
    int thisRelFrame = useSavedFrame ? mSavedRelFrame : mRelFrame;
    qreal startRelFrame = useSavedFrame ? mSavedStartFrame : mStartFrame;
    qreal endRelFrame = useSavedFrame ? mSavedEndFrame : mEndFrame;

    setStartFrameVar(thisRelFrame + (startRelFrame - thisRelFrame)*scaleFactor);
    setEndFrameVar(thisRelFrame + (endRelFrame - thisRelFrame)*scaleFactor);

    int relativeToRelFrame =
            mParentAnimator->prp_absFrameToRelFrame(relativeToFrame);
    int newFrame = qRound(relativeToRelFrame +
                          (thisRelFrame - relativeToRelFrame)*scaleFactor);
    if(newFrame != mRelFrame) {
        incFrameAndUpdateParentAnimator(newFrame - mRelFrame);
    } else {
        mParentAnimator->anim_updateAfterChangedKey(this);
    }
}

qreal QrealKey::getStartValue() const {
    if(mStartEnabled) return mStartValue;
    return mValue;
}

qreal QrealKey::getEndValue() const {
    if(mEndEnabled) return mEndValue;
    return mValue;
}

qreal QrealKey::getStartValueFrame() const {
    if(mStartEnabled) return mStartFrame;
    return mRelFrame;
}

qreal QrealKey::getEndValueFrame() const {
    if(mEndEnabled) return mEndFrame;
    return mRelFrame;
}

//void QrealKey::makeStartAndEndSmooth() {
//    qreal nextKeyVal = getNextKeyValue();
//    qreal prevKeyVal = getPrevKeyValue();
//    int nextKeyFrame = getNextKeyRelFrame();
//    int prevKeyFrame = getPrevKeyRelFrame();
//    qreal valIncPerFrame;
//    if(nextKeyFrame == mRelFrame || prevKeyFrame == mRelFrame) {
//        valIncPerFrame = 0.;
//    } else {
//        valIncPerFrame =
//                (nextKeyVal - prevKeyVal)/(nextKeyFrame - prevKeyFrame);
//    }
//    mStartValue = mValue + (mStartFrame - mRelFrame)*valIncPerFrame;
//    mEndValue = mValue + (mEndFrame - mRelFrame)*valIncPerFrame;
//}

void QrealKey::setStartEnabledForGraph(const bool &bT) {
    mStartEnabled = bT;
}

void QrealKey::setEndEnabledForGraph(const bool &bT) {
    mEndEnabled = bT;
}

void QrealKey::saveCurrentFrameAndValue() {
    mSavedRelFrame = getRelFrame();
    mSavedValue = getValue();
}

void QrealKey::setRelFrame(const int &frame) {
    if(frame == mRelFrame) return;
    int dFrame = frame - mRelFrame;
    setEndFrameVar(mEndFrame + dFrame);
    setStartFrameVar(mStartFrame + dFrame);
    mRelFrame = frame;
    if(mParentAnimator == nullptr) return;
    mParentAnimator->anim_updateKeyOnCurrrentFrame();
}

bool QrealKey::differsFromKey(Key *key) const {
    if(key == this) return false;
    stdsptr<QrealKey> qa_key = GetAsSPtr(key, QrealKey);
    if(isZero4Dec(qa_key->getValue() - mValue)) {
        if(key->getRelFrame() > mRelFrame) {
            if(qa_key->getStartEnabledForGraph() || mEndEnabled) return true;
        } else {
            if(qa_key->getEndEnabledForGraph() || mStartEnabled) return true;
        }
        return false;
    }
    return true;
}

void QrealKey::changeFrameAndValueBy(const QPointF &frameValueChange) {
    int newFrame = qRound(frameValueChange.x() + mSavedRelFrame);
    bool frameChanged = newFrame != mRelFrame;
    setValue(frameValueChange.y() + mSavedValue, false, !frameChanged);
    if(!frameChanged) return;
    if(mParentAnimator != nullptr) {
        mParentAnimator->anim_moveKeyToRelFrame(this, newFrame, false);
    } else {
        setRelFrame(newFrame);
    }
}
