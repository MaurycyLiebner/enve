#include "animator.h"
#include "Animators/complexanimator.h"
#include "mainwindow.h"

Animator::Animator() :
    QObject(),
    ConnectedToMainWindow(),
    SingleWidgetTarget() {
    setFrame(mMainWindow->getCurrentFrame());
}

void Animator::setFrame(int frame) {
    mCurrentFrame = frame;
}

void Animator::switchRecording() {
    setRecording(!mIsRecording);
}

void Animator::setParentAnimator(ComplexAnimator *parentAnimator) {
    mParentAnimator = parentAnimator;
    if(parentAnimator == NULL) {
        clearFromGraphView();
    }
}

void Animator::setZValue(const int &oldIndex,
                         const int &newIndex) {
    if(mParentAnimator == NULL) return;
    mParentAnimator->changeChildAnimatorZ(oldIndex, newIndex);
}
