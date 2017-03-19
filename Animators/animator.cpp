#include "animator.h"
#include "Animators/complexanimator.h"
#include "mainwindow.h"

Animator::Animator() :
    Property() {
}

void Animator::setFrame(int frame) {
    mCurrentFrame = frame;
}

void Animator::switchRecording() {
    setRecording(!mIsRecording);
}

int Animator::getFrameShift() const {
    if(mParentAnimator == NULL) {
        return 0;
    }
    return mParentAnimator->getFrameShift();
}
