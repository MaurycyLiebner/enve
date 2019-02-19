#include "animator.h"

#include <QPainter>
#include "Animators/complexanimator.h"
#include "key.h"
#include "fakecomplexanimator.h"
#include "PropertyUpdaters/propertyupdater.h"
#include "qrealpoint.h"

Animator::Animator(const QString& name) : Property(name) {}

void Animator::scaleTime(const int &pivotAbsFrame, const qreal &scale) {
    for(const auto &key : anim_mKeys) {
        key->scaleFrameAndUpdateParentAnimator(pivotAbsFrame, scale, false);
    }
}

void Animator::anim_shiftAllKeys(const int &shift) {
    for(const auto &key : anim_mKeys) {
        anim_moveKeyToRelFrame(key.get(),
                               key->getRelFrame() + shift);
    }
}

bool Animator::prp_nextRelFrameWithKey(const int &relFrame,
                                       int &nextRelFrame) {
    Key *key = anim_getNextKey(relFrame);
    if(!key) return false;
    nextRelFrame = key->getRelFrame();
    return true;
}

bool Animator::prp_prevRelFrameWithKey(const int &relFrame,
                                       int &prevRelFrame) {
    Key *key = anim_getPrevKey(relFrame);
    if(!key) return false;
    prevRelFrame = key->getRelFrame();
    return true;
}

stdsptr<Key> Animator::readKey(QIODevice *target) {
    Q_UNUSED(target);
    return nullptr;
}

bool Animator::hasFakeComplexAnimator() {
    return !mFakeComplexAnimator.isNull();
}

FakeComplexAnimator *Animator::getFakeComplexAnimator() {
    return mFakeComplexAnimator.data();
}

void Animator::enableFakeComplexAnimator() {
    if(!mFakeComplexAnimator.isNull()) return;
    SWT_hide();
    mFakeComplexAnimator = SPtrCreate(FakeComplexAnimator)(prp_mName, this);
    emit prp_prependWith(this, mFakeComplexAnimator);
}

void Animator::disableFakeComplexAnimator() {
    if(mFakeComplexAnimator.isNull()) return;
    SWT_show();
    emit mFakeComplexAnimator->prp_replaceWith(mFakeComplexAnimator,
                                               nullptr);
    mFakeComplexAnimator.reset();
}

void Animator::disableFakeComplexAnimatrIfNotNeeded() {
    if(mFakeComplexAnimator.isNull()) return;
    if(mFakeComplexAnimator->ca_getNumberOfChildren() == 0) {
        disableFakeComplexAnimator();
    }
}

int Animator::anim_getPrevKeyRelFrame(const Key * const key) const {
    if(!key) return FrameRange::EMIN;
    Key * const prevKey = key->getPrevKey();
    if(!prevKey) return FrameRange::EMIN;
    return prevKey->getRelFrame();
}

int Animator::anim_getNextKeyRelFrame(const Key * const key) const {
    if(!key) return FrameRange::EMAX;
    Key * const nextKey = key->getNextKey();
    if(!nextKey) return FrameRange::EMAX;
    return nextKey->getRelFrame();
}

int Animator::anim_getPrevKeyRelFrame(const int &relFrame) const {
    Key * const prevKey = anim_getPrevKey(relFrame);
    if(!prevKey) return FrameRange::EMIN;
    return prevKey->getRelFrame();
}

int Animator::anim_getNextKeyRelFrame(const int &relFrame) const {
    Key * const nextKey = anim_getNextKey(relFrame);
    if(!nextKey) return FrameRange::EMAX;
    return nextKey->getRelFrame();
}

void Animator::prp_updateAfterChangedAbsFrameRange(const FrameRange &range) {
    if(range.inRange(anim_mCurrentAbsFrame)) {
        prp_currentFrameChanged();
    }
    emit prp_absFrameRangeChanged(range);
}

void Animator::anim_updateAfterChangedKey(Key *key) {
    if(anim_mIsComplexAnimator) return;
    if(!key) {
        prp_updateInfluenceRangeAfterChanged();
        return;
    }
    int prevKeyRelFrame = anim_getPrevKeyRelFrame(key);
    if(prevKeyRelFrame != FrameRange::EMIN) prevKeyRelFrame++;
    int nextKeyRelFrame = anim_getNextKeyRelFrame(key);
    if(nextKeyRelFrame != FrameRange::EMAX) nextKeyRelFrame--;
    prp_updateAfterChangedRelFrameRange({prevKeyRelFrame, nextKeyRelFrame});
}

void Animator::prp_setAbsFrame(const int &frame) {
    anim_mCurrentAbsFrame = frame;
    anim_updateRelFrame();
    anim_updateKeyOnCurrrentFrame();
    //anim_callFrameChangeUpdater();
}

void Animator::anim_updateRelFrame() {
    anim_mCurrentRelFrame = anim_mCurrentAbsFrame - prp_getFrameShift();
}

void Animator::prp_setRecording(const bool &rec) {
    if(rec) {
        anim_setRecordingWithoutChangingKeys(rec);
        anim_saveCurrentValueAsKey();
    } else {
        anim_removeAllKeys();
        anim_setRecordingWithoutChangingKeys(rec);
    }
}

void Animator::prp_switchRecording() {
    prp_setRecording(!anim_mIsRecording);
}

bool Animator::prp_isDescendantRecording() const { return anim_mIsRecording; }

bool Animator::anim_isComplexAnimator() { return anim_mIsComplexAnimator; }

void Animator::prp_startDragging() {}

void Animator::anim_mergeKeysIfNeeded() {
    Key* lastKey = nullptr;
    QList<KeyPair> keyPairsToMerge;
    for(const auto &key : anim_mKeys) {
        Key* keyPtr = key.get();
        if(lastKey) {
            if(keyPtr->getAbsFrame() == lastKey->getAbsFrame() ) {
                if(keyPtr->isDescendantSelected()) {
                    keyPairsToMerge << KeyPair(keyPtr, lastKey);
                } else {
                    keyPairsToMerge << KeyPair(lastKey, keyPtr);
                    keyPtr = nullptr;
                }
            }
        }
        lastKey = keyPtr;
    }
    for(const KeyPair& keyPair : keyPairsToMerge) {
        keyPair.merge();
    }
}

bool Animator::anim_getClosestsKeyOccupiedRelFrame(const int &frame,
                                                   int &closest) {
    int nextT;
    const bool hasNext = prp_nextRelFrameWithKey(frame, nextT);
    int prevT;
    const bool hasPrev = prp_prevRelFrameWithKey(frame, prevT);
    if(hasPrev && hasNext) {
        if(nextT - frame > frame - prevT) {
            closest = prevT;
        } else {
            closest = nextT;
        }
    } else if(hasNext) {
        closest = nextT;
    } else if(hasPrev) {
        closest = prevT;
    } else {
        return false;
    }
    return true;
}

Key *Animator::anim_getKeyAtAbsFrame(const int &frame) {
    return anim_getKeyAtRelFrame(prp_absFrameToRelFrame(frame));
}

void Animator::anim_saveCurrentValueAsKey() {}

void Animator::anim_addKeyAtRelFrame(const int &relFrame) { Q_UNUSED(relFrame); }

int Animator::anim_getKeyIndex(const Key * const key) const {
    int index = -1;
    for(int i = 0; i < anim_mKeys.count(); i++) {
        if(anim_mKeys.at(i).get() == key) {
            index = i;
        }
    }
    return index;
}

Key* Animator::anim_getNextKey(const Key * const key) const {
    const int keyId = anim_getKeyIndex(key);
    if(keyId == anim_mKeys.count() - 1) return nullptr;
    return anim_mKeys.at(keyId + 1).get();
}

Key* Animator::anim_getPrevKey(const Key * const key) const {
    const int keyId = anim_getKeyIndex(key);
    if(keyId <= 0) return nullptr;
    return anim_mKeys.at(keyId - 1).get();
}

std::pair<Key*, Key*> Animator::anim_getPrevAndNextKey(const int &relFrame) const {
    std::pair<Key*, Key*> result(nullptr, nullptr);
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(prevId, nextId, relFrame)) {
        Key* prevKey = anim_mKeys.at(prevId).get();
        if(prevKey->getRelFrame() < relFrame) {
            result.first = prevKey;
        } else if(prevKey->getRelFrame() == relFrame) {
            if(prevId > 0) {
                result.first = anim_mKeys.at(prevId - 1).get();
            }
        }
        Key* nextKey = anim_mKeys.at(nextId).get();
        if(nextKey->getRelFrame() > relFrame) {
            result.second = prevKey;
        } else if(nextKey->getRelFrame() == relFrame) {
            if(nextId + 1 < anim_mKeys.count()) {
                result.second = anim_mKeys.at(nextId + 1).get();
            }
        }
    }
    return result;
}

Key *Animator::anim_getPrevKey(const int &relFrame) const {
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(prevId, nextId, relFrame)) {
        Key* key = anim_mKeys.at(prevId).get();
        if(key->getRelFrame() < relFrame) {
            return key;
        } else if(key->getRelFrame() == relFrame) {
            if(prevId > 0) {
                return anim_mKeys.at(prevId - 1).get();
            }
        }
    }
    return nullptr;
}

Key *Animator::anim_getNextKey(const int &relFrame) const {
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(prevId, nextId, relFrame)) {
        Key* key = anim_mKeys.at(nextId).get();
        if(key->getRelFrame() > relFrame) {
            return key;
        } else if(key->getRelFrame() == relFrame) {
            if(nextId + 1 < anim_mKeys.count()) {
                return anim_mKeys.at(nextId + 1).get();
            }
        }
    }
    return nullptr;
}

void Animator::anim_deleteCurrentKey() {
    if(anim_mKeyOnCurrentFrame) anim_mKeyOnCurrentFrame->deleteKey();
}

Key *Animator::anim_getKeyAtRelFrame(const int &frame) const {
    if(anim_mKeys.isEmpty()) return nullptr;
    if(frame > anim_mKeys.last()->getRelFrame()) return nullptr;
    if(frame < anim_mKeys.first()->getRelFrame()) return nullptr;
    int minId = 0;
    int maxId = anim_mKeys.count() - 1;
    while(maxId - minId > 1) {
        const int guess = (maxId + minId)/2;
        if(anim_mKeys.at(guess)->getRelFrame() > frame) {
            maxId = guess;
        } else {
            minId = guess;
        }
    }
    Key* minIdKey = anim_mKeys.at(minId).get();
    if(minIdKey->getRelFrame() == frame) return minIdKey;
    Key* maxIdKey = anim_mKeys.at(maxId).get();
    if(maxIdKey->getRelFrame() == frame) return maxIdKey;
    return nullptr;
}

bool Animator::anim_hasPrevKey(const Key * const key) {
    int keyId = anim_getKeyIndex(key);
    if(keyId > 0) return true;
    return false;
}

bool Animator::anim_hasNextKey(const Key * const key) {
    int keyId = anim_getKeyIndex(key);
    if(keyId < anim_mKeys.count() - 1) return true;
    return false;
}

void Animator::anim_callFrameChangeUpdater() {
    if(prp_mUpdater) prp_mUpdater->frameChangeUpdate();
}

void Animator::anim_updateAfterShifted() {
    for(const auto &key : anim_mKeys) {
        emit prp_removingKey(key.get());
        emit prp_addingKey(key.get());
    }
}

bool keysFrameSort(const stdsptr<Key> &key1,
                   const stdsptr<Key> &key2) {
    return key1->getAbsFrame() < key2->getAbsFrame();
}

void Animator::anim_sortKeys() {
    std::sort(anim_mKeys.begin(), anim_mKeys.end(), keysFrameSort);
}

int Animator::getInsertIdForKeyRelFrame(const int& relFrame) const {
    return getInsertIdForKeyRelFrame(relFrame, 0, anim_mKeys.count() - 1);
}

int Animator::getInsertIdForKeyRelFrame(
        const int& relFrame, const int& min, const int& max) const {
    if(min == max) return min;
    const int guess = (max + min)/2;
    const Key * const key = anim_mKeys.at(guess).get();
    const int guessFrame = key->getRelFrame();
    if(guessFrame > relFrame) {
        return getInsertIdForKeyRelFrame(relFrame, min, guess);
    } else if(guessFrame < relFrame) {
        return getInsertIdForKeyRelFrame(relFrame, guess + 1, max);
    }
    // guessFrame == relFrame
    return guess;
}

void Animator::anim_afterInsertedKey(const int& insertId,
                                     Key * const insertedKey) const {
    const auto prevKey = insertId == 0 ?
                nullptr : anim_mKeys.at(insertId - 1).get();
    const auto nextKey = insertId >= anim_mKeys.count() ?
                nullptr : anim_mKeys.at(insertId + 1).get();
    if(prevKey) prevKey->updateAfterNextKeyChanged(insertedKey);
    insertedKey->updateAfterNeighbouringKeysChanged(prevKey, nextKey);
    if(nextKey) nextKey->updateAfterPrevKeyChanged(insertedKey);
}

void Animator::anim_appendKey(const stdsptr<Key>& newKey) {
    if(!anim_mIsRecording) anim_mIsRecording = true;
    const int insertId = getInsertIdForKeyRelFrame(newKey->getRelFrame());
    anim_mKeys.insert(insertId, newKey);
    anim_afterInsertedKey(insertId, newKey.get());
    //anim_sortKeys();
    //mergeKeysIfNeeded();
    emit prp_addingKey(newKey.get());
    anim_updateKeyOnCurrrentFrame();
    anim_updateAfterChangedKey(newKey.get());
}

void Animator::anim_afterRemovedKey(const int& removedId) const {
    const auto prevKey = removedId == 0 ?
                nullptr : anim_mKeys.at(removedId - 1).get();
    const auto nextKey = removedId >= anim_mKeys.count() ?
                nullptr : anim_mKeys.at(removedId).get();
    if(prevKey) prevKey->updateAfterNextKeyChanged(nextKey);
    if(nextKey) nextKey->updateAfterPrevKeyChanged(prevKey);
}

void Animator::anim_removeKey(const stdsptr<Key>& keyToRemove) {
    Key * const keyPtr = keyToRemove.get();
    anim_updateAfterChangedKey(keyPtr);
    emit prp_removingKey(keyPtr);
    const int keyId = anim_getKeyIndex(keyPtr);
    anim_mKeys.removeAt(keyId);
    anim_afterRemovedKey(keyId);
    //anim_sortKeys();
    anim_updateKeyOnCurrrentFrame();
}

void Animator::anim_moveKeyToRelFrame(Key *key, const int &newFrame) {
    const auto keySPtr = GetAsSPtr(key, Key);
    anim_removeKey(keySPtr);
    key->setRelFrame(newFrame);
    anim_appendKey(keySPtr);
}

void Animator::anim_keyValueChanged(Key *key) {
    anim_updateAfterChangedKey(key);
}

void Animator::anim_updateKeyOnCurrrentFrame() {
    anim_mKeyOnCurrentFrame = anim_getKeyAtAbsFrame(anim_mCurrentAbsFrame);
}

DurationRectangleMovable *Animator::anim_getRectangleMovableAtPos(
        const int &relX, const int &minViewedFrame, const qreal &pixelsPerFrame) {
    Q_UNUSED(relX);
    Q_UNUSED(minViewedFrame);
    Q_UNUSED(pixelsPerFrame);
    return nullptr;
}

Key *Animator::prp_getKeyAtPos(const qreal &relX,
                               const int &minViewedFrame,
                               const qreal &pixelsPerFrame,
                               const int& keyRectSize) {
    qreal relFrame = relX/pixelsPerFrame - 0.5 - prp_getFrameShift();
    qreal absX = relX + minViewedFrame*pixelsPerFrame;
    qreal absFrame = relFrame + minViewedFrame;
    qreal keySize = keyRectSize;
    if(SWT_isComplexAnimator()) {
        keySize *= 0.75;
    }
    if(pixelsPerFrame > keySize) {
        int relFrameInt = qRound(relFrame);
        qreal distToFrame = qAbs((relFrameInt + 0.5)*pixelsPerFrame - relX);
        if(2*distToFrame > keySize) {
            return nullptr;
        }
    }
    //if(pressFrame < 0) pressFrame -= 1.;
    qreal keyRectFramesSpan = 0.5*keySize/pixelsPerFrame;
    int minPossibleKey = qFloor(absFrame - keyRectFramesSpan);
    int maxPossibleKey = qCeil(absFrame + keyRectFramesSpan);
    Key* keyAtPos = nullptr;
    for(int i = maxPossibleKey; i >= minPossibleKey; i--) {
        qreal distToFrame = qAbs((i + 0.5)*pixelsPerFrame - absX);
        if(2*distToFrame > keySize) continue;
        keyAtPos = anim_getKeyAtRelFrame(i);
        if(keyAtPos) return keyAtPos;
    }
    return nullptr;
}

void Animator::prp_addAllKeysToComplexAnimator(ComplexAnimator *target) {
    for(const auto &key : anim_mKeys) {
        target->ca_addDescendantsKey(key.get());
    }
}

void Animator::prp_removeAllKeysFromComplexAnimator(ComplexAnimator *target) {
    for(const auto &key : anim_mKeys) {
        target->ca_removeDescendantsKey(key.get());
    }
}

bool Animator::prp_hasKeys() const {
    return !anim_mKeys.isEmpty();
}

void Animator::anim_setRecordingWithoutChangingKeys(const bool &rec) {
    anim_setRecordingValue(rec);
}

void Animator::anim_setRecordingValue(const bool &rec) {
    if(rec == anim_mIsRecording) return;
    anim_mIsRecording = rec;
    emit prp_isRecordingChanged();
}

bool Animator::SWT_isAnimator() const { return true; }

bool Animator::prp_isRecording() {
    return anim_mIsRecording;
}

void Animator::anim_removeAllKeys() {
    if(anim_mKeys.isEmpty()) return;
    QList<stdsptr<Key>> keys = anim_mKeys;
    for(const auto& key : keys) {
        anim_removeKey(key);
    }
}

bool Animator::prp_isKeyOnCurrentFrame() const {
    return anim_mKeyOnCurrentFrame != nullptr;
}

void Animator::prp_getKeysInRect(const QRectF &selectionRect,
                                 const qreal &pixelsPerFrame,
                                 QList<Key*> &keysList,
                                 const int& keyRectSize) {
    //selectionRect.translate(-getFrameShift(), 0.);
    int selLeftFrame = qRound(selectionRect.left());
    if(0.5*pixelsPerFrame + keyRectSize*0.5 <
       selectionRect.left() - selLeftFrame*pixelsPerFrame) {
        selLeftFrame++;
    }
    int selRightFrame = qRound(selectionRect.right());
    if(0.5*pixelsPerFrame - keyRectSize*0.5 >
       selectionRect.right() - selRightFrame*pixelsPerFrame) {
        selRightFrame--;
    }
    for(int i = selRightFrame; i >= selLeftFrame; i--) {
        Key* keyAtPos = anim_getKeyAtAbsFrame(i);
        if(keyAtPos) {
            keysList.append(keyAtPos);
        }
    }
}

bool Animator::anim_getNextAndPreviousKeyIdForRelFrame(
                        int &prevIdP, int &nextIdP,
                        const int &frame) const {
    if(anim_mKeys.isEmpty()) return false;
    int minId = 0;
    int maxId = anim_mKeys.count() - 1;
    if(frame >= anim_mKeys.last()->getRelFrame()) {
        prevIdP = maxId;
        nextIdP = maxId;
        return true;
    }
    if(frame <= anim_mKeys.first()->getRelFrame()) {
        prevIdP = minId;
        nextIdP = minId;
        return true;
    }
    while(maxId - minId > 1) {
        const int guess = (maxId + minId)/2;
        const int keyFrame = anim_mKeys.at(guess)->getRelFrame();
        if(keyFrame > frame) {
            maxId = guess;
        } else if(keyFrame < frame) {
            minId = guess;
        } else {
            nextIdP = guess;
            prevIdP = guess;
            return true;
        }
    }

    if(minId == maxId) {
        const auto key = anim_mKeys.at(minId).get();
        if(key->getRelFrame() > frame) {
            if(minId != 0) {
                minId = minId - 1;
            }
        } else if(key->getRelFrame() < frame) {
            if(minId < anim_mKeys.count() - 1) {
                maxId = minId + 1;
            }
        }
    }
    prevIdP = minId;
    nextIdP = maxId;
    return true;
}

bool Animator::anim_getNextAndPreviousKeyIdForRelFrameF(
                        int &prevIdP, int &nextIdP,
                        const qreal &frame) const {
    if(anim_mKeys.isEmpty()) return false;
    int minId = 0;
    int maxId = anim_mKeys.count() - 1;
    if(frame + 0.0001 >= anim_mKeys.last()->getRelFrame()) {
        prevIdP = maxId;
        nextIdP = maxId;
        return true;
    }
    if(frame + 0.0001 <= anim_mKeys.first()->getRelFrame()) {
        prevIdP = minId;
        nextIdP = minId;
        return true;
    }
    while(maxId - minId > 1) {
        const int guess = (maxId + minId)/2;
        const int keyFrame = anim_mKeys.at(guess)->getRelFrame();
        if(keyFrame > frame) {
            maxId = guess;
        } else if(keyFrame < frame) {
            minId = guess;
        } else {
            nextIdP = guess;
            prevIdP = guess;
            return true;
        }
    }

    if(minId == maxId) {
        const auto key = anim_mKeys.at(minId).get();
        if(key->getRelFrame() > frame) {
            if(minId != 0) {
                minId = minId - 1;
            }
        } else if(key->getRelFrame() < frame) {
            if(minId < anim_mKeys.count() - 1) {
                maxId = minId + 1;
            }
        }
    }
    prevIdP = minId;
    nextIdP = maxId;
    return true;
}

int Animator::anim_getCurrentAbsFrame() {
    return anim_mCurrentAbsFrame;
}

int Animator::anim_getCurrentRelFrame() {
    return anim_mCurrentRelFrame;
}

FrameRange Animator::prp_getIdenticalRelFrameRange(const int &relFrame) const {
    if(anim_mKeys.isEmpty()) {
        return {FrameRange::EMIN, FrameRange::EMAX};
    } else {
        int prevId;
        int nextId;
        anim_getNextAndPreviousKeyIdForRelFrame(prevId, nextId, relFrame);

        Key* prevKey = anim_mKeys.at(prevId).get();
        Key* nextKey = anim_mKeys.at(nextId).get();
        Key* prevPrevKey = nextKey;
        Key* prevNextKey = prevKey;

        int fId = relFrame;
        int lId = relFrame;

        while(true) {
            if(prevKey->differsFromKey(prevPrevKey)) break;
            fId = prevKey->getRelFrame();
            prevPrevKey = prevKey;
            prevKey = prevKey->getPrevKey();
            if(!prevKey) {
                fId = FrameRange::EMIN;
                break;
            }
        }

        while(true) {
            if(nextKey->differsFromKey(prevNextKey)) break;
            lId = nextKey->getRelFrame();
            prevNextKey = nextKey;
            nextKey = nextKey->getNextKey();
            if(!nextKey) {
                lId = FrameRange::EMAX;
                break;
            }
        }

        return {fId, lId};
    }
}

void Animator::anim_drawKey(QPainter *p,
                            Key *key,
                            const qreal &pixelsPerFrame,
                            const qreal &drawY,
                            const int &startFrame,
                            const int &rowHeight,
                            const int &keyRectSize) {
    if(key->isSelected()) {
        p->setBrush(Qt::yellow);
    } else {
        p->setBrush(Qt::red);
    }
    if(key->isHovered()) {
        p->setPen(QPen(Qt::black, 1.5));
    } else {
        p->setPen(QPen(Qt::black, .75));
    }
    p->drawEllipse(
        QRectF(
            QPointF((key->getRelFrame() - startFrame + 0.5)*
                    pixelsPerFrame - keyRectSize*0.5,
                    drawY + (rowHeight -
                              keyRectSize)*0.5 ),
            QSize(keyRectSize, keyRectSize) ) );
}

void Animator::prp_drawKeys(QPainter *p,
                            const qreal &pixelsPerFrame,
                            const qreal &drawY,
                            const int &startFrame,
                            const int &endFrame,
                            const int &rowHeight,
                            const int &keyRectSize) {
    p->save();
    p->translate(prp_getFrameShift()*pixelsPerFrame, 0.);
    for(const auto &key : anim_mKeys) {
        if(key->getAbsFrame() >= startFrame &&
           key->getAbsFrame() <= endFrame) {
            anim_drawKey(p, key.get(), pixelsPerFrame,
                         drawY, startFrame,
                         rowHeight, keyRectSize);
        }
    }
    p->restore();
}


bool Animator::hasSelectedKeys() const {
    return !anim_mSelectedKeys.isEmpty();
}

void Animator::addKeyToSelected(Key *key) {
    anim_mSelectedKeys << key;
    key->setSelected(true);
}

void Animator::removeKeyFromSelected(Key *key) {
    key->setSelected(false);
    anim_mSelectedKeys.removeOne(key);
}

void Animator::sortSelectedKeys() {
//    std::sort(anim_mSelectedKeys.begin(),
//              anim_mSelectedKeys.end(),
//              keysFrameSort);
}

void Animator::deselectAllKeys() {
    for(const auto& key : anim_mSelectedKeys) {
        key->setSelected(false);
    }
    anim_mSelectedKeys.clear();
}

void Animator::selectAllKeys() {
    for(const auto& key : anim_mKeys) {
        anim_mSelectedKeys.append(key.get());
        key->setSelected(true);
    }
}

void Animator::incSelectedKeysFrame(const int &dFrame) {
    for(const auto& key : anim_mSelectedKeys) {
        key->incFrameAndUpdateParentAnimator(dFrame);
    }
}

void Animator::scaleSelectedKeysFrame(const int &absPivotFrame,
                                      const qreal &scale) {
    for(const auto& key : anim_mSelectedKeys) {
        key->scaleFrameAndUpdateParentAnimator(absPivotFrame, scale, true);
    }
}

void Animator::cancelSelectedKeysTransform() {
    for(const auto& key : anim_mSelectedKeys) {
        key->cancelFrameTransform();
    }
}

void Animator::finishSelectedKeysTransform() {
    for(const auto& key : anim_mSelectedKeys) {
        key->finishFrameTransform();
    }
}

void Animator::startSelectedKeysTransform() {
    for(const auto& key : anim_mSelectedKeys) {
        key->startFrameTransform();
    }
}

void Animator::deleteSelectedKeys() {
    for(const auto& key : anim_mSelectedKeys) {
        key->deleteKey();
    }
    anim_mSelectedKeys.clear();
}

int Animator::getLowestAbsFrameForSelectedKey() {
    int lowestKey = FrameRange::EMAX;
    for(const auto& key : anim_mSelectedKeys) {
        int keyAbsFrame = key->getAbsFrame();
        if(keyAbsFrame < lowestKey) {
            lowestKey = keyAbsFrame;
        }
    }
    return lowestKey;
}
