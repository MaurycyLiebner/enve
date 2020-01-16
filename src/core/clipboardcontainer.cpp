// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "clipboardcontainer.h"
#include "Boxes/boundingbox.h"
#include "Boxes/containerbox.h"
#include "Properties/boxtargetproperty.h"
#include "PathEffects/patheffectcollection.h"
#include "Animators/SmartPath/smartpathcollection.h"
#include "Properties/boxtargetproperty.h"

Clipboard::Clipboard(const ClipboardType type) : mType(type) {}

ClipboardType Clipboard::getType() const { return mType; }

BoxesClipboard::BoxesClipboard(const QList<BoundingBox*> &src) :
    Clipboard(ClipboardType::boxes) {
    QBuffer buffer(&mData);
    buffer.open(QIODevice::WriteOnly);
    eWriteStream writeStream(&buffer);
    const int nBoxes = src.count();
    writeStream << nBoxes;

    const bool isBox = true;
    const int iCount = src.count();
    for(int i = iCount - 1; i >= 0; i--) {
        const auto& iBox = src[i];
        const auto future = writeStream.planFuturePos();
        writeStream << isBox;
        iBox->writeIdentifier(writeStream);
        iBox->writeBoundingBox(writeStream);
        writeStream.writeCheckpoint();
        writeStream.assignFuturePos(future);
    }
    writeStream.writeFutureTable();
    buffer.close();

    BoundingBox::sClearWriteBoxes();
}

#include "canvas.h"
void BoxesClipboard::pasteTo(ContainerBox* const parent) {
    const int oldCount = parent->getContainedBoxesCount();
    QBuffer buffer(&mData);
    buffer.open(QIODevice::ReadOnly);
    eReadStream readStream(&buffer);
    buffer.seek(buffer.size() - qint64(sizeof(int)));
    readStream.readFutureTable();
    buffer.seek(0);
    try {
        parent->readAllContained(readStream);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
    buffer.close();
    const int newCount = parent->getContainedBoxesCount();
    const auto parentScene = parent->getParentScene();
    if(parentScene) {
        const auto& list = parent->getContainedBoxes();
        for(int i = 0; i < newCount - oldCount; i++) {
            const auto& box = list.at(i);
            parentScene->addBoxToSelection(box);
        }
    }
    BoundingBox::sClearReadBoxes();
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
        QBuffer buffer(const_cast<QByteArray*>(&animData.second));
        buffer.open(QIODevice::ReadOnly);
        eReadStream readStream(&buffer);
        int nKeys; readStream >> nKeys;
        for(int i = 0; i < nKeys; i++) {
            const auto keyT = animator->anim_createKey();
            keyT->readKey(readStream);
            if(keyT->getAbsFrame() < firstKeyFrame)
                firstKeyFrame = keyT->getAbsFrame();
            keys << keyT;
        }
        buffer.close();

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
    mContentType(std::type_index(typeid(*source))) {
    QBuffer buffer(&mData);
    buffer.open(QIODevice::WriteOnly);
    eWriteStream writeStream(&buffer);
    source->prp_writeProperty(writeStream);
    buffer.close();
}

bool PropertyClipboard::paste(Property * const target) {
    if(!compatibleTarget(target)) return false;
    QBuffer buffer(&mData);
    buffer.open(QIODevice::ReadOnly);
    eReadStream readStream(&buffer);
    target->prp_readProperty(readStream);
    buffer.close();
    return true;
}
