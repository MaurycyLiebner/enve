#include "animator.h"
#include "Animators/complexanimator.h"
#include "mainwindow.h"

Animator::Animator() :
    Property() {
}

void Animator::setAbsFrame(int frame) {
    mCurrentAbsFrame = frame;
    updateRelFrame();
}

void Animator::updateRelFrame() {
    mCurrentRelFrame = mCurrentAbsFrame - getFrameShift();
}

int Animator::absFrameToRelFrame(const int &absFrame) const {
    return absFrame - getFrameShift();
}

int Animator::relFrameToAbsFrame(const int &relFrame) const {
    return relFrame + getFrameShift();
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
