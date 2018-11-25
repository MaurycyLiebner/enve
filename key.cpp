#include "key.h"

#include "Animators/qrealanimator.h"
#include "Animators/complexanimator.h"
#include "clipboardcontainer.h"

Key::Key(Animator* parentAnimator) {
    mParentAnimator = parentAnimator;
    mRelFrame = 0;
}

Key::~Key() {}

bool Key::isSelected() { return mIsSelected; }

void Key::removeFromAnimator() {
    if(mParentAnimator == nullptr) return;
    mParentAnimator->anim_removeKey(ref<Key>());
}

Key* Key::getNextKey() {
    return mParentAnimator->anim_getNextKey(this);
}

Key* Key::getPrevKey() {
    return mParentAnimator->anim_getPrevKey(this);
}

bool Key::hasPrevKey() {
    if(mParentAnimator == nullptr) return false;
    return mParentAnimator->anim_hasPrevKey(this);
}

bool Key::hasNextKey() {
    if(mParentAnimator == nullptr) return false;
    return mParentAnimator->anim_hasNextKey(this);
}

void Key::incFrameAndUpdateParentAnimator(const int &inc,
                                          const bool &finish) {
    setRelFrameAndUpdateParentAnimator(mRelFrame + inc, finish);
}

void Key::setRelFrameAndUpdateParentAnimator(const int &relFrame,
                                          const bool &finish) {
    if(mParentAnimator == nullptr) return;
    mParentAnimator->anim_moveKeyToRelFrame(this, relFrame, finish);
}

void Key::addToSelection(QList<Key*> &selectedKeys) {
    if(isSelected()) return;
    setSelected(true);
    selectedKeys.append(this);
}

void Key::removeFromSelection(QList<Key*> &selectedKeys) {
    if(isSelected()) {
        setSelected(false);
        selectedKeys.removeOne(this);
    }
}

Animator* Key::getParentAnimator() {
    return mParentAnimator;
}

void Key::startFrameTransform() {
    mSavedRelFrame = mRelFrame;
}

void Key::cancelFrameTransform() {
    mParentAnimator->anim_moveKeyToRelFrame(this,
                                            mSavedRelFrame,
                                            false);
}

void Key::scaleFrameAndUpdateParentAnimator(
        const int &relativeToFrame,
        const qreal &scaleFactor,
        const bool &useSavedFrame) {
    int thisRelFrame = useSavedFrame ? mSavedRelFrame : mRelFrame;
    int relativeToRelFrame =
            mParentAnimator->prp_absFrameToRelFrame(relativeToFrame);
    int newFrame = qRound(relativeToRelFrame +
                          (thisRelFrame - relativeToRelFrame)*scaleFactor);
    if(newFrame == mRelFrame) return;
    setRelFrameAndUpdateParentAnimator(newFrame);
}

void Key::setSelected(const bool &bT) {
    mIsSelected = bT;
}
#include "undoredo.h"
void Key::finishFrameTransform() {
    if(mParentAnimator == nullptr) return;
//    mParentAnimator->addUndoRedo(
//                new ChangeKeyFrameUndoRedo(mSavedRelFrame,
//                                           mRelFrame, this));
}

int Key::getAbsFrame() {
    return mParentAnimator->prp_relFrameToAbsFrame(mRelFrame);
}

int Key::getRelFrame() {
    return mRelFrame;
}

void Key::setRelFrame(const int &frame) {
    if(frame == mRelFrame) return;
    mRelFrame = frame;
    if(mParentAnimator == nullptr) return;
    mParentAnimator->anim_updateKeyOnCurrrentFrame();
}

void Key::setAbsFrame(const int &frame) {
    setRelFrame(mParentAnimator->prp_absFrameToRelFrame(frame));
}
