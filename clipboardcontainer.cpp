#include "clipboardcontainer.h"
#include "Boxes/boundingbox.h"
#include "Boxes/boxesgroup.h"

ClipboardContainer::ClipboardContainer(const ClipboardContainerType &type) {
    mType = type;
}

ClipboardContainerType ClipboardContainer::getType() {
    return mType;
}

BoxesClipboardContainer::BoxesClipboardContainer() :
    ClipboardContainer(CCT_BOXES) {

}

BoxesClipboardContainer::~BoxesClipboardContainer() {
    foreach(BoundingBox *box, mBoxesList) {
        box->decNumberPointers();
    }
}

void BoxesClipboardContainer::copyBoxToContainer(BoundingBox *box) {
    mBoxesList << box->createDuplicate(NULL);
    box->incNumberPointers();
}

void BoxesClipboardContainer::pasteTo(BoxesGroup *parent) {
    foreach(BoundingBox *box, mBoxesList) {
        box->createDuplicate(parent);
    }
}

KeysClipboardContainer::KeysClipboardContainer() :
    ClipboardContainer(CCT_KEYS) {

}

KeysClipboardContainer::~KeysClipboardContainer() {
    foreach(QrealAnimator *animator, mTargetAnimators) {
        animator->decNumberPointers();
    }
    foreach(QrealKey *key, mKeysList) {
        key->decNumberPointers();
    }
}

void KeysClipboardContainer::copyKeyToContainer(QrealKey *key) {
    mKeysList << key->makeQrealKeyDuplicate(NULL);
    mTargetAnimators << key->getParentAnimator();
    key->getParentAnimator()->incNumberPointers();
}

#include "keysview.h"
void KeysClipboardContainer::paste(const int &pasteFrame,
                                   KeysView *keysView) {
    int firstKeyFrame = 1000000;
    foreach(QrealKey *key, mKeysList) {
        if(key->getAbsFrame() < firstKeyFrame) {
            firstKeyFrame = key->getAbsFrame();
        }
    }
    int dFrame = pasteFrame - firstKeyFrame;

    QList<QrealAnimator*> animators;
    foreach(QrealAnimator *animator, mTargetAnimators) {
        if(animators.contains(animator)) continue;
        animators << animator;
    }

    keysView->clearKeySelection();

    int count = mKeysList.count();
    for(int i = 0; i < count; i++) {
        QrealKey *key = mKeysList.at(i);
        QrealAnimator *animator = mTargetAnimators.at(i);
        key->setRelFrame(key->getAbsFrame() + dFrame);
        key = key->makeQrealKeyDuplicate(animator);
        animator->appendKey(key);
        keysView->addKeyToSelection(key);
    }

    foreach(QrealAnimator *animator, animators) {
        animator->mergeKeysIfNeeded();
    }
}

AnimatorClipboardContainer::AnimatorClipboardContainer() :
    ClipboardContainer(CCT_ANIMATOR) {

}

AnimatorClipboardContainer::~AnimatorClipboardContainer() {
    mAnimator->decNumberPointers();
}

void AnimatorClipboardContainer::setAnimator(QrealAnimator *animator) {
    mAnimator = animator;
    mAnimator->incNumberPointers();
}

void AnimatorClipboardContainer::paste(QrealAnimator *target) {
    QString nameT = target->getName();
    if(nameT == mAnimator->getName() ||
       !(target->isComplexAnimator() || mAnimator->isComplexAnimator())) {
        mAnimator->makeDuplicate(target);
        target->setName(nameT);
    }
}
