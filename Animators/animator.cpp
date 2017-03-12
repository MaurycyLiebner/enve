#include "animator.h"
#include "Animators/complexanimator.h"
#include "mainwindow.h"

Animator::Animator() :
    Property() {
    setFrame(mMainWindow->getCurrentFrame());
}

void Animator::setFrame(int frame) {
    mCurrentFrame = frame;
}

void Animator::switchRecording() {
    setRecording(!mIsRecording);
}
