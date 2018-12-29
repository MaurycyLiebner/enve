#include "qrealkey.h"
#include "Animators/qrealanimator.h"
#include "Animators/complexanimator.h"
#include "clipboardcontainer.h"
#include "Animators/qrealpoint.h"

QrealKey::QrealKey(const int &frame,
                   const qreal &val,
                   QrealAnimator* parentAnimator) :
    GraphKey(frame, parentAnimator) {
    mValue = val;
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

QrealAnimator *QrealKey::getParentQrealAnimator() const {
    return static_cast<QrealAnimator*>(mParentAnimator.data());
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

void QrealKey::saveCurrentFrameAndValue() {
    mSavedRelFrame = getRelFrame();
    mSavedValue = getValue();
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
