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

KeysClipboardContainer::KeysClipboardContainer() {

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

void KeysClipboardContainer::paste(const int &pasteFrame) {
    int firstKeyFrame = 1000000;
    foreach(QrealKey *key, mKeysList) {
        if(key->getFrame() < firstKeyFrame) {
            firstKeyFrame = key->getFrame();
        }
    }
    int dFrame = pasteFrame - firstKeyFrame;

    int count = mKeysList.count();
    for(int i = 0; i < count; i++) {
        QrealKey *key = mKeysList.at(i);
        QrealAnimator *animator = mTargetAnimators.at(i);
        key = key->makeQrealKeyDuplicate(NULL);

        key->setFrame(key->getFrame() + dFrame);
        animator->appendKey(key);
    }
}
