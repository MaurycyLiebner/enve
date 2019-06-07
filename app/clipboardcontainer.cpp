#include "clipboardcontainer.h"
#include "Boxes/boundingbox.h"
#include "Boxes/containerbox.h"
#include "Properties/boxtargetproperty.h"
#include "PathEffects/patheffectanimators.h"
#include "Animators/SmartPath/smartpathcollection.h"
#include "Properties/boxtargetproperty.h"
#include "GUI/keysview.h"
#include "Animators/effectanimators.h"
#include "castmacros.h"

ClipboardContainer::ClipboardContainer(const ClipboardContainerType &type) {
    mType = type;
}

ClipboardContainer::~ClipboardContainer() {}

ClipboardContainerType ClipboardContainer::getType() {
    return mType;
}

QByteArray *ClipboardContainer::getBytesArray() {
    return &mData;
}

BoxesClipboardContainer::BoxesClipboardContainer() :
    ClipboardContainer(CCT_BOXES) {}
#include "canvas.h"
void BoxesClipboardContainer::pasteTo(ContainerBox* const parent) {
    const int oldCount = parent->getContainedBoxesCount();
    QBuffer target(getBytesArray());
    target.open(QIODevice::ReadOnly);
    parent->readChildBoxes(&target);
    target.close();
    BoundingBox::sClearReadBoxes();
    const int newCount = parent->getContainedBoxesCount();
    const auto parentCanvas = parent->getParentCanvas();
    if(parentCanvas) {
        const auto& list = parent->getContainedBoxesList();
        for(int i = oldCount; i < newCount; i++) {
            const auto& box = list.at(i);
            parentCanvas->addBoxToSelection(box.get());
        }
    }
}

KeysClipboardContainer::KeysClipboardContainer() :
    ClipboardContainer(CCT_KEYS) {

}

KeysClipboardContainer::~KeysClipboardContainer() {

}

void KeysClipboardContainer::paste(const int pasteFrame,
                                   KeysView * const keysView,
                                   const bool merge,
                                   const bool selectPasted) {
    keysView->clearKeySelection();

    QList<Key*> rKeys;
    int firstKeyFrame = FrameRange::EMAX;

    QList<QList<stdsptr<Key>>> animatorKeys;
    for(const auto &animData : mAnimatorData) {
        Animator *animator = animData.first;
        if(!animator) continue;
        QList<stdsptr<Key>> keys;
        int nKeys;
        QBuffer dst(const_cast<QByteArray*>(&animData.second));
        dst.open(QIODevice::ReadOnly);
        dst.read(rcChar(&nKeys), sizeof(int));
        for(int i = 0; i < nKeys; i++) {
            const auto keyT = animator->readKey(&dst);
            if(keyT->getAbsFrame() < firstKeyFrame)
                firstKeyFrame = keyT->getAbsFrame();
            keys << keyT;
        }
        dst.close();

        animatorKeys << keys;
    }
    if(firstKeyFrame == FrameRange::EMAX) return;
    int dFrame = pasteFrame - firstKeyFrame;

    int keysId = 0;
    for(const auto &animData : mAnimatorData) {
        Animator * const animator = animData.first;
        const auto& keys = animatorKeys.at(keysId);
        for(const auto& key : keys) {
            key->setRelFrame(key->getRelFrame() + dFrame);
            animator->anim_appendKey(key);
            rKeys.append(key.get());
            if(selectPasted) keysView->addKeyToSelection(key.get());
        }
        if(merge) animator->anim_mergeKeysIfNeeded();
        keysId++;
    }
}

void KeysClipboardContainer::addTargetAnimator(
        Animator *anim, const QByteArray &keyData) {
    mAnimatorData << AnimatorKeyDataPair(QPointer<Animator>(anim), keyData);
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
                    GetAsPtr(targetProperty, ComplexAnimator)->
                            ca_removeAllChildAnimators();
                }
            } else {
                GetAsPtr(targetProperty, Animator)->anim_removeAllKeys();
            }
        }
    }
    paste(targetProperty);
}

void PropertyClipboardContainer::paste(Property *targetProperty) {
    QBuffer target(getBytesArray());
    target.open(QIODevice::ReadOnly);
    if(propertyCompatible(targetProperty)) {
        if(mBoxTargetProperty) {
            if(targetProperty->SWT_isBoxTargetProperty()) {
                GetAsPtr(targetProperty, BoxTargetProperty)->setTarget(
                            mTargetBox.data());
            }
        } else if(mPathEffect &&
                  targetProperty->SWT_isPathEffectAnimators()) {
            GetAsPtr(targetProperty, PathEffectAnimators)->readPathEffect(&target);
        } else if(mPixmapEffect &&
                  targetProperty->SWT_isPixmapEffectAnimators()) {
            GetAsPtr(targetProperty, EffectAnimators)->readPixmapEffect(&target);
        } else if(mPathEffectAnimators &&
                  targetProperty->SWT_isPathEffectAnimators()) {
            GetAsPtr(targetProperty, PathEffectAnimators)->readProperty(&target);
        } else if(mPixmapEffectAnimators &&
                  targetProperty->SWT_isPixmapEffectAnimators()) {
            GetAsPtr(targetProperty, EffectAnimators)->readProperty(&target);
        } else if(mVectorPathAnimator &&
                  targetProperty->SWT_isSmartPathCollection()) {
            RuntimeThrow("NO CODE");
            //GetAsPtr(targetProperty, SmartPathCollection)->(&target);
        } else {
            targetProperty->readProperty(&target);
        }
    }
    target.close();
}

bool PropertyClipboardContainer::propertyCompatible(Property *target) {
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
        return target->SWT_isSmartPathCollection();
    }
    if(mVectorPathAnimator) {
        return target->SWT_isSmartPathCollection() ||
                target->SWT_isSmartPathAnimator();
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
    if(mPathEffect) {
        return target->SWT_isPathEffectAnimators();
    }
    if(mPixmapEffect) {
        return target->SWT_isPixmapEffectAnimators();
    }
    if(mComplexAnimator) {
        return target->SWT_isComplexAnimator() &&
                target->prp_getName() == mPropertyName;
    }
    return false;
}

void PropertyClipboardContainer::setProperty(Property *property) {
    QBuffer targetBuff(getBytesArray());
    targetBuff.open(QIODevice::WriteOnly);
    property->writeProperty(&targetBuff);
    targetBuff.close();
    mPropertyName = property->prp_getName();
    mQrealAnimator = property->SWT_isQrealAnimator();
    mQPointFAnimator = property->SWT_isQPointFAnimator();
    mQStringAnimator = property->SWT_isQStringAnimator();
    mPathAnimator = property->SWT_isSmartPathCollection();
    mAnimatedSurface = property->SWT_isAnimatedSurface();
    mComplexAnimator = property->SWT_isComplexAnimator();
    mPathEffectAnimators = property->SWT_isPathEffectAnimators();
    mPixmapEffectAnimators = property->SWT_isPixmapEffectAnimators();
    mPathEffect = property->SWT_isPathEffect();
    mVectorPathAnimator = property->SWT_isSmartPathAnimator();
    mPixmapEffect = property->SWT_isPixmapEffect();
    mBoxTargetProperty = property->SWT_isBoxTargetProperty();
    if(mBoxTargetProperty) {
        mTargetBox = GetAsPtr(property, BoxTargetProperty)->getTarget();
    }
}
