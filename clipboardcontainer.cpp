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
    foreach(Animator *animator, mTargetAnimators) {
        animator->decNumberPointers();
    }
    foreach(KeyCloner *keyCloner, mKeyClonersList) {
        delete keyCloner;
    }
}

void KeysClipboardContainer::copyKeyToContainer(Key *key) {
    mKeyClonersList << key->createNewKeyCloner();
    mTargetAnimators << key->getParentAnimator();
    key->getParentAnimator()->incNumberPointers();
}

#include "keysview.h"
void KeysClipboardContainer::paste(const int &pasteFrame,
                                   KeysView *keysView) {
    int firstKeyFrame = 1000000;
    foreach(KeyCloner *keyCloner, mKeyClonersList) {
        if(keyCloner->getAbsFrame() < firstKeyFrame) {
            firstKeyFrame = keyCloner->getAbsFrame();
        }
    }
    int dFrame = pasteFrame - firstKeyFrame;

    QList<Animator*> animators;
    foreach(Animator *animator, mTargetAnimators) {
        if(animators.contains(animator)) continue;
        animators << animator;
    }

    keysView->clearKeySelection();

    int count = mKeyClonersList.count();
    for(int i = 0; i < count; i++) {
        KeyCloner *keyCloner = mKeyClonersList.at(i);
        Animator *animator = mTargetAnimators.at(i);
        keyCloner->shiftKeyFrame(dFrame);
        Key *newKey = keyCloner->createKeyForAnimator(animator);
        keysView->addKeyToSelection(newKey);
    }

    foreach(Animator *animator, animators) {
        animator->anim_mergeKeysIfNeeded();
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
    QString nameT = target->prp_getName();
    if(nameT == mAnimator->prp_getName() ||
       !(target->anim_isComplexAnimator() ||
         mAnimator->anim_isComplexAnimator())) {
        mAnimator->prp_makeDuplicate(target);
        target->prp_setName(nameT);
    }
}
