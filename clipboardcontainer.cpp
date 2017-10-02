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
}

void BoxesClipboardContainer::copyBoxToContainer(BoundingBox *box) {
    mBoxesList << box->createDuplicate();
}

void BoxesClipboardContainer::pasteTo(BoxesGroup *parent) {
    QBuffer target(getBytesArray());
    target.open(QIODevice::ReadOnly);
    parent->readChildBoxes(&target);
    target.close();
}

KeysClipboardContainer::KeysClipboardContainer() :
    ClipboardContainer(CCT_KEYS) {

}

KeysClipboardContainer::~KeysClipboardContainer() {

}

#include "keysview.h"
void KeysClipboardContainer::paste(const int &pasteFrame,
                                   KeysView *keysView) {
    keysView->clearKeySelection();

    int firstKeyFrame = 1000000;
    QBuffer target(getBytesArray());
    target.open(QIODevice::ReadOnly);
    QList<Key*> keys;
    Q_FOREACH(const QWeakPointer<Animator> &animatorT, mTargetAnimators) {
        Animator *animator = animatorT.data();
        Key *keyT = animator->readKey(&target);
        if(animator == NULL) {
            keyT->ref<Key>();
            continue;
        }
        if(keyT->getAbsFrame() < firstKeyFrame) {
            firstKeyFrame = keyT->getAbsFrame();
        }
        keys << keyT;
    }
    target.close();
    int dFrame = pasteFrame - firstKeyFrame;

    int keyId = 0;
    QList<Animator*> animators;

    Q_FOREACH(const QWeakPointer<Animator> &animatorT, mTargetAnimators) {
        Animator *animator = animatorT.data();
        if(animator == NULL) {
            continue;
        }
        Key *keyT = keys.at(keyId);
        keyT->setRelFrame(keyT->getRelFrame() + dFrame);
        animator->anim_appendKey(keyT);
        keyId++;
        if(animators.contains(animator)) continue;
        animators << animator;
    }

    Q_FOREACH(Animator *animator, animators) {
        animator->anim_mergeKeysIfNeeded();
    }
}

void KeysClipboardContainer::addTargetAnimator(Animator *anim) {
    mTargetAnimators << anim->weakRef<Animator>();
}

PropertyClipboardContainer::PropertyClipboardContainer() :
    ClipboardContainer(CCT_PROPERTY) {

}

PropertyClipboardContainer::~PropertyClipboardContainer() {
}

void PropertyClipboardContainer::clearAndPaste(Property *targetProperty) {
    QBuffer target(getBytesArray());
    target.open(QIODevice::ReadOnly);
    if(propertyCompatible(targetProperty)) {
        if(targetProperty->SWT_isAnimator()) {
            if(targetProperty->SWT_isComplexAnimator()) {
                if(targetProperty->SWT_isEffectAnimators()) {
                    ((ComplexAnimator*)targetProperty)->ca_removeAllChildAnimators();
                }
            } else {
                ((Animator*)targetProperty)->anim_removeAllKeys();
            }
        }
        targetProperty->readProperty(&target);
        targetProperty->prp_callUpdater();
        targetProperty->prp_callFinishUpdater();
    }
    target.close();
}

void PropertyClipboardContainer::paste(Property *targetProperty) {
    QBuffer target(getBytesArray());
    target.open(QIODevice::ReadOnly);
    if(propertyCompatible(targetProperty)) {
        targetProperty->readProperty(&target);
        targetProperty->prp_callUpdater();
        targetProperty->prp_callFinishUpdater();
    }
    target.close();
}

bool PropertyClipboardContainer::propertyCompatible(Property *target) {
    QString nameT = target->prp_getName();
    if(mQrealAnimator) {
        return target->SWT_isQrealAnimator();
    }
    if(mQPointFAnimator) {
        return target->SWT_isQPointFAnimator();
    }
    if(mQStringAnimator) {
        return target->SWT_isQStringAnimator();
    }
    if(mPathAnimator) {
        return target->SWT_isVectorPathAnimator();
    }
    if(mAnimatedSurface) {
        return target->SWT_isAnimatedSurface();
    }
    if(mComplexAnimator) {
        return target->SWT_isComplexAnimator() &&
                nameT == mPropertyName;
    }
    return false;
}

void PropertyClipboardContainer::setProperty(Property *property) {
    QBuffer targetBuff(getBytesArray());
    targetBuff.open(QIODevice::WriteOnly);
    property->writeProperty(&targetBuff);
    targetBuff.close();
    mQrealAnimator = property->SWT_isQrealAnimator();
    mQPointFAnimator = property->SWT_isQPointFAnimator();
    mQStringAnimator = property->SWT_isQStringAnimator();
    mPathAnimator = property->SWT_isPathAnimator();
    mAnimatedSurface = property->SWT_isAnimatedSurface();
    mComplexAnimator = property->SWT_isComplexAnimator();
}
