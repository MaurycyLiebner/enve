#include "clipboardcontainer.h"
#include "Boxes/boundingbox.h"
#include "Boxes/containerbox.h"
#include "Properties/boxtargetproperty.h"
#include "PathEffects/patheffectanimators.h"
#include "Animators/SmartPath/smartpathcollection.h"
#include "Properties/boxtargetproperty.h"
#include "castmacros.h"

Clipboard::Clipboard(const ClipboardType type) : mType(type) {}

ClipboardType Clipboard::getType() const { return mType; }

BoxesClipboard::BoxesClipboard(const QList<BoundingBox*> &src) :
    Clipboard(ClipboardType::boxes) {
    QBuffer dst(&mData);
    dst.open(QIODevice::WriteOnly);
    const int nBoxes = src.count();
    dst.write(rcConstChar(&nBoxes), sizeof(int));

    for(const auto& box : src) {
        box->writeBoxType(&dst);
        box->writeBoundingBox(&dst);
    }
    dst.close();

    BoundingBox::sClearWriteBoxes();
}

#include "canvas.h"
void BoxesClipboard::pasteTo(ContainerBox* const parent) {
    const int oldCount = parent->getContainedBoxesCount();
    QBuffer src(&mData);
    src.open(QIODevice::ReadOnly);
    parent->readChildBoxes(&src);
    src.close();
    BoundingBox::sClearReadBoxes();
    const int newCount = parent->getContainedBoxesCount();
    const auto parentCanvas = parent->getParentScene();
    if(parentCanvas) {
        const auto& list = parent->getContainedBoxes();
        for(int i = oldCount; i < newCount; i++) {
            const auto& box = list.at(i);
            parentCanvas->addBoxToSelection(box.get());
        }
    }
}

KeysClipboard::KeysClipboard() : Clipboard(ClipboardType::keys) {}

void KeysClipboard::paste(const int pasteFrame, const bool merge,
                          const std::function<void(Key*)>& selectAction) {
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
            const auto keyT = animator->createKey();
            keyT->readKey(&dst);
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
            if(selectAction) selectAction(key.get());
        }
        if(merge) animator->anim_mergeKeysIfNeeded();
        keysId++;
    }
}

void KeysClipboard::addTargetAnimator(
        Animator *anim, const QByteArray &keyData) {
    mAnimatorData << AnimatorKeyDataPair(QPointer<Animator>(anim), keyData);
}

PropertyClipboard::PropertyClipboard(const Property* const source) :
    Clipboard(ClipboardType::property),
    mContentType(std::type_index(typeid(source))) {
    QBuffer dst(&mData);
    dst.open(QIODevice::WriteOnly);
    source->writeProperty(&dst);
    dst.close();
}

bool PropertyClipboard::paste(Property * const target) {
    if(!compatibleTarget(target)) return false;
    QBuffer src(&mData);
    src.open(QIODevice::ReadOnly);
    target->readProperty(&src);
    src.close();
    return true;
}
