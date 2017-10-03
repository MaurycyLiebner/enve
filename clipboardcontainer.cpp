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
    if(propertyCompatible(targetProperty)) {
        if(targetProperty->SWT_isAnimator()) {
            if(targetProperty->SWT_isComplexAnimator()) {
                if(targetProperty->SWT_isPixmapEffectAnimators() ||
                   targetProperty->SWT_isPathEffectAnimators()) {
                    ((ComplexAnimator*)targetProperty)->
                            ca_removeAllChildAnimators();
                }
            } else {
                ((Animator*)targetProperty)->anim_removeAllKeys();
            }
        }
    }
    paste(targetProperty);
}
#include "PathEffects/patheffectanimators.h"
#include "Animators/pathanimator.h"
#include "Properties/boxtargetproperty.h"
void PropertyClipboardContainer::paste(Property *targetProperty) {
    QBuffer target(getBytesArray());
    target.open(QIODevice::ReadOnly);
    if(propertyCompatible(targetProperty)) {
        if(mBoxTargetProperty) {
            if(targetProperty->SWT_isBoxTargetProperty()) {
                ((BoxTargetProperty*)targetProperty)->setTarget(
                            mTargetBox.data());
            }
        } else if(mPathEffect &&
           targetProperty->SWT_isPathEffectAnimators()) {
            ((PathEffectAnimators*)targetProperty)->readPathEffect(&target);
        } else if(mPixmapEffect &&
            targetProperty->SWT_isPixmapEffectAnimators()) {
            ((EffectAnimators*)targetProperty)->readPixmapEffect(&target);
        } else if(mPathEffectAnimators &&
            targetProperty->SWT_isPathEffectAnimators()) {
            ((PathEffectAnimators*)targetProperty)->readProperty(&target);
        } else if(mPixmapEffectAnimators &&
            targetProperty->SWT_isPixmapEffectAnimators()) {
            ((EffectAnimators*)targetProperty)->readProperty(&target);
        } else if(mVectorPathAnimator &&
            targetProperty->SWT_isPathAnimator()) {
            ((PathAnimator*)targetProperty)->readVectorPathAnimator(&target);
        } else {
            targetProperty->readProperty(&target);
        }
        targetProperty->prp_callUpdater();
        targetProperty->prp_callFinishUpdater();
    }
    target.close();
}

bool PropertyClipboardContainer::propertyCompatible(Property *target) {
    QString nameT = target->prp_getName();
    if(mBoxTargetProperty) {
        return target->SWT_isBoxTargetProperty();
    }
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
        return target->SWT_isPathAnimator();
    }
    if(mVectorPathAnimator) {
        return target->SWT_isPathAnimator() ||
                target->SWT_isVectorPathAnimator();
    }
    if(mAnimatedSurface) {
        return target->SWT_isAnimatedSurface();
    }
    if(mPathEffectAnimators) {
        return target->SWT_isPathEffectAnimators();
    }
    if(mPixmapEffectAnimators) {
        return target->SWT_isPixmapEffectAnimators();
    }
    if(target->SWT_isPathEffectAnimators()) {
        return mPathEffect;
    }
    if(target->SWT_isPixmapEffectAnimators()) {
        return mPixmapEffect;
    }
    if(mComplexAnimator) {
        return target->SWT_isComplexAnimator() &&
                nameT == mPropertyName;
    }
    return false;
}
#include "Properties/boxtargetproperty.h"
void PropertyClipboardContainer::setProperty(Property *property) {
    QBuffer targetBuff(getBytesArray());
    targetBuff.open(QIODevice::WriteOnly);
    property->writeProperty(&targetBuff);
    targetBuff.close();
    mPropertyName = property->prp_getName();
    mQrealAnimator = property->SWT_isQrealAnimator();
    mQPointFAnimator = property->SWT_isQPointFAnimator();
    mQStringAnimator = property->SWT_isQStringAnimator();
    mPathAnimator = property->SWT_isPathAnimator();
    mAnimatedSurface = property->SWT_isAnimatedSurface();
    mComplexAnimator = property->SWT_isComplexAnimator();
    mPathEffectAnimators = property->SWT_isPathEffectAnimators();
    mPixmapEffectAnimators = property->SWT_isPixmapEffectAnimators();
    mPathEffect = property->SWT_isPathEffect();
    mVectorPathAnimator = property->SWT_isVectorPathAnimator();
    mPixmapEffect = property->SWT_isPixmapEffect();
    mBoxTargetProperty = property->SWT_isBoxTargetProperty();
    if(mBoxTargetProperty) {
        BoundingBox *targetBox = ((BoxTargetProperty*)property)->getTarget();
        if(targetBox == NULL) {
            mTargetBox.clear();
        } else {
            mTargetBox = targetBox->weakRef<BoundingBox>();
        }
    }
}
