#include "clipboardcontainer.h"
#include "Boxes/boundingbox.h"
#include "Boxes/boxesgroup.h"
#include "Properties/boxtargetproperty.h"
#include "PathEffects/patheffectanimators.h"
#include "Animators/pathanimator.h"
#include "Properties/boxtargetproperty.h"
#include "GUI/keysview.h"
#include "Animators/effectanimators.h"

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

void BoxesClipboardContainer::pasteTo(BoxesGroup* parent) {
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

void KeysClipboardContainer::paste(const int &pasteFrame,
                                   KeysView *keysView,
                                   const bool& merge,
                                   const bool& selectPasted) {
    keysView->clearKeySelection();

    QList<Key*> rKeys;
    int firstKeyFrame = INT_MAX;

    QList<QList<stdsptr<Key>>> animatorKeys;
    Q_FOREACH(const auto &animData, mAnimatorData) {
        Animator *animator = animData.first;
        if(animator == nullptr) continue;
        QList<stdsptr<Key>> keys;
        int nKeys;
        QBuffer target(const_cast<QByteArray*>(&animData.second));
        target.open(QIODevice::ReadOnly);
        target.read(reinterpret_cast<char*>(&nKeys), sizeof(int));
        for(int i = 0; i < nKeys; i++) {
            stdsptr<Key> keyT = animator->readKey(&target);
            if(keyT->getAbsFrame() < firstKeyFrame) {
                firstKeyFrame = keyT->getAbsFrame();
            }
            keys << keyT;
        }
        target.close();

        animatorKeys << keys;
    }
    if(firstKeyFrame == INT_MAX) return;
    int dFrame = pasteFrame - firstKeyFrame;

    int keysId = 0;
    Q_FOREACH(const auto &animData, mAnimatorData) {
        Animator *animator = animData.first;
        const QList<stdsptr<Key>>& keys = animatorKeys.at(keysId);
        Q_FOREACH(const auto& key, keys) {
            key->setRelFrame(key->getRelFrame() + dFrame);
            animator->anim_appendKey(key);
            rKeys.append(key.get());
            if(selectPasted) keysView->addKeyToSelection(key.get());
        }
        keysId++;
    }
    if(merge) {
        Q_FOREACH(const auto &animData, mAnimatorData) {
            Animator *animator = animData.first;
            if(animator == nullptr) continue;
            animator->anim_mergeKeysIfNeeded();
        }
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
            targetProperty->SWT_isPathAnimator()) {
            GetAsPtr(targetProperty, PathAnimator)->readVectorPathAnimator(&target);
        } else {
            targetProperty->readProperty(&target);
        }
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
        mTargetBox = GetAsPtr(property, BoxTargetProperty)->getTarget();
    }
}
