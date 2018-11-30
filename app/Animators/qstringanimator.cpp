#include "qstringanimator.h"
#include "undoredo.h"

QStringAnimator::QStringAnimator(const QString &name) :
    Animator(name) {}

void QStringAnimator::prp_setAbsFrame(const int &frame) {
    Animator::prp_setAbsFrame(frame);
    if(prp_hasKeys()) {
        mCurrentText = getTextValueAtRelFrame(anim_mCurrentRelFrame);
    }
}

void QStringAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) prp_setRecording(true);

    if(anim_mKeyOnCurrentFrame == nullptr) {
        auto newKey = SPtrCreate(QStringKey)(mCurrentText,
                                             anim_mCurrentRelFrame,
                                             this);
        anim_appendKey(newKey);
        anim_mKeyOnCurrentFrame = newKey.get();
    } else {
        GetAsPtr(anim_mKeyOnCurrentFrame, QStringKey)->setText(mCurrentText);
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
    Key *key;
    if(prp_isKeyOnCurrentFrame()) {
        key = anim_mKeyOnCurrentFrame;
    } else {
        key = anim_getPrevKey(relFrame);
    }
    if(key == nullptr) {
        key = anim_getNextKey(relFrame);
    }
    return GetAsPtr(key, QStringKey)->getText();
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
            if(prevKey == nullptr) {
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
            if(nextKey == nullptr) {
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
    return GetAsPtr(key, QStringKey)->getText() != mText;
}
