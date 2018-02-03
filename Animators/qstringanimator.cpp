#include "qstringanimator.h"
#include "undoredo.h"

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

void QStringAnimator::setCurrentTextValue(const QString &text,
                                          const bool &saveUndoRedo) {
    if(saveUndoRedo) {
//        addUndoRedo(new ChangeTextUndoRedo(this,
//                                           mCurrentText,
//                                           text));
    }
    mCurrentText = text;
    if(prp_isRecording() && saveUndoRedo) {
        anim_saveCurrentValueAsKey();
    } else {
        prp_updateInfluenceRangeAfterChanged();
    }

    prp_callUpdater();
}

QString QStringAnimator::getCurrentTextValue() {
    return mCurrentText;
}

QString QStringAnimator::getTextValueAtRelFrame(const int &relFrame) {
    if(anim_mKeys.isEmpty()) {
        return mCurrentText;
    }
    QStringKey *key;
    if(prp_isKeyOnCurrentFrame()) {
        key = (QStringKey*)anim_mKeyOnCurrentFrame;
    } else {
        key = (QStringKey*)anim_getPrevKey(relFrame);
    }
    if(key == NULL) {
        key = (QStringKey*)anim_getNextKey(relFrame);
    }
    return key->getText();
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
        Key *prevPrevKey = nextKey;
        Key *prevNextKey = prevKey;

        int fId = relFrame;
        int lId = relFrame;

        while(true) {
            fId = prevKey->getRelFrame();
            prevPrevKey = prevKey;
            prevKey = prevKey->getPrevKey();
            if(prevKey == NULL) {
                fId = INT_MIN;
                break;
            }
            if(prevKey->differsFromKey(prevPrevKey)) break;
        }

        while(true) {
            lId = nextKey->getRelFrame();
            if(nextKey->differsFromKey(prevNextKey)) break;
            prevNextKey = nextKey;
            nextKey = nextKey->getNextKey();
            if(nextKey == NULL) {
                lId = INT_MAX;
                break;
            }
        }
        *firstIdentical = fId;
        *lastIdentical = lId;
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
