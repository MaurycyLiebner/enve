#include "qstringanimator.h"

QStringAnimator::QStringAnimator() : Animator() {

}

void QStringAnimator::prp_setAbsFrame(const int &frame) {
    Animator::prp_setAbsFrame(frame);
    if(prp_hasKeys()) {
        mCurrentText = getTextValueAtRelFrame(anim_mCurrentRelFrame);
    }
}

void QStringAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) prp_setRecording(true);

    if(anim_mKeyOnCurrentFrame == NULL) {
        anim_mKeyOnCurrentFrame = new QStringKey(mCurrentText,
                                                 anim_mCurrentRelFrame,
                                                 this);
        anim_appendKey(anim_mKeyOnCurrentFrame);
    } else {
        ((QStringKey*)anim_mKeyOnCurrentFrame)->setText(mCurrentText);
    }
}

void QStringAnimator::anim_loadKeysFromSql(const int &qrealAnimatorId) {

}

void QStringAnimator::setCurrentTextValue(const QString &text) {
    mCurrentText = text;
    if(prp_isRecording()) {
        anim_saveCurrentValueAsKey();
    }
}

QString QStringAnimator::getCurrentTextValue() {
    return mCurrentText;
}

QString QStringAnimator::getTextValueAtRelFrame(const int &relFrame) {
    QStringKey *key = (QStringKey *)anim_getPrevKey(relFrame);
    if(key == NULL) {
        return mCurrentText;
    }
    return key->getText();
}

void QStringAnimator::prp_setRecording(const bool &rec) {
    if(rec) {
        anim_setRecordingWithoutChangingKeys(rec);
        anim_saveCurrentValueAsKey();
    } else {
        anim_removeAllKeys();
        anim_setRecordingWithoutChangingKeys(rec);
    }
}

void QStringAnimator::prp_getFirstAndLastIdenticalRelFrame(
                            int *firstIdentical,
                            int *lastIdentical,
                            const int &relFrame) {
    if(anim_mKeys.isEmpty()) {
        *firstIdentical = INT_MIN;
        *lastIdentical = INT_MAX;
    } else {
        int prevId;
        int nextId;
        anim_getNextAndPreviousKeyIdForRelFrame(&prevId, &nextId,
                                                relFrame);
        Key *prevKey = anim_mKeys.at(prevId).get();
        Key *nextKey = anim_mKeys.at(nextId).get();
        if(prevId == nextId) {
            if(prevKey->getNextKey() == NULL) {
                *firstIdentical = nextKey->getRelFrame();
                *lastIdentical = INT_MAX;
            } else if(nextKey->getPrevKey() == NULL) {
                *firstIdentical = INT_MIN;
                *lastIdentical = nextKey->getRelFrame();
            } else {
                *firstIdentical = prevKey->getRelFrame();
                *lastIdentical = prevKey->getNextKey()->getRelFrame();
            }
        } else {
            *firstIdentical = prevKey->getRelFrame();
            *lastIdentical = nextKey->getRelFrame();
        }
    }
}

QStringKey::QStringKey(const QString &stringT,
                       const int &relFrame,
                       QStringAnimator *parentAnimator) :
    Key(parentAnimator) {
    mRelFrame = relFrame;
    mText = stringT;
}

bool QStringKey::differsFromKey(Key *key) {
    return ((QStringKey*)key)->getText() != mText;
}
