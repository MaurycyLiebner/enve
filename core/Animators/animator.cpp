#include "animator.h"

#include <QPainter>
#include "Animators/complexanimator.h"
#include "key.h"
#include "fakecomplexanimator.h"
#include "PropertyUpdaters/propertyupdater.h"
#include "qrealpoint.h"

Animator::Animator(const QString& name) : Property(name) {}

void Animator::anim_scaleTime(const int &pivotAbsFrame, const qreal &scale) {
    for(const auto &key : anim_mKeys) {
        key->scaleFrameAndUpdateParentAnimator(pivotAbsFrame, scale, false);
    }
}

void Animator::anim_shiftAllKeys(const int &shift) {
    for(const auto &key : anim_mKeys) {
        anim_moveKeyToRelFrame(key.get(), key->getRelFrame() + shift);
    }
}

bool Animator::anim_nextRelFrameWithKey(const int &relFrame,
                                       int &nextRelFrame) {
    const auto key = anim_getNextKey(relFrame);
    if(!key) return false;
    nextRelFrame = key->getRelFrame();
    return true;
}

bool Animator::anim_prevRelFrameWithKey(const int &relFrame,
                                       int &prevRelFrame) {
    const auto key = anim_getPrevKey(relFrame);
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
    if(range.inRange(anim_mCurrentAbsFrame)) prp_currentFrameChanged();
    emit prp_absFrameRangeChanged(range);
}

void Animator::anim_updateAfterChangedKey(Key * const key) {
    if(SWT_isComplexAnimator()) return;
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

void Animator::anim_setAbsFrame(const int &frame) {
    anim_mCurrentAbsFrame = frame;
    anim_updateRelFrame();
    anim_updateKeyOnCurrrentFrame();
    //anim_callFrameChangeUpdater();
}

void Animator::anim_updateRelFrame() {
    anim_mCurrentRelFrame = anim_mCurrentAbsFrame - prp_getFrameShift();
}

void Animator::anim_setRecording(const bool &rec) {
    if(rec == anim_mIsRecording) return;
    if(rec) {
        anim_setRecordingWithoutChangingKeys(rec);
        anim_saveCurrentValueAsKey();
    } else {
        anim_removeAllKeys();
        anim_setRecordingWithoutChangingKeys(rec);
    }
}

void Animator::anim_switchRecording() {
    anim_setRecording(!anim_mIsRecording);
}

bool Animator::anim_isDescendantRecording() const { return anim_mIsRecording; }

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
    const bool hasNext = anim_nextRelFrameWithKey(frame, nextT);
    int prevT;
    const bool hasPrev = anim_prevRelFrameWithKey(frame, prevT);
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

bool Animator::anim_hasPrevKey(const Key * const key) {
    const int keyId = anim_getKeyIndex(key);
    if(keyId > 0) return true;
    return false;
}

bool Animator::anim_hasNextKey(const Key * const key) {
    const int keyId = anim_getKeyIndex(key);
    if(keyId < anim_mKeys.count() - 1) return true;
    return false;
}

int Animator::anim_getPrevKeyId(const int &relFrame) const {
    return anim_getPrevAndNextKeyIdForRelFrame(relFrame).first;
}

int Animator::anim_getNextKeyId(const int &relFrame) const {
    return anim_getPrevAndNextKeyIdForRelFrame(relFrame).second;
}


int Animator::anim_getKeyIndex(const Key * const key) const {
    int index = -1;
    for(int i = 0; i < anim_mKeys.count(); i++) {
        if(anim_mKeys.at(i).get() == key) {
            index = i;
        }
    }
    return index;
}

void Animator::anim_coordinateKeysWith(Animator * const other) {
    for(const auto& otherKey : other->anim_mKeys) {
        const int absFrame = otherKey->getAbsFrame();
        const int relFrame = prp_absFrameToRelFrame(absFrame);
        if(!anim_getKeyAtRelFrame(relFrame))
            anim_addKeyAtRelFrame(relFrame);
    }

    for(const auto& thisKey : anim_mKeys) {
        const int absFrame = thisKey->getAbsFrame();
        const int relFrame = other->prp_absFrameToRelFrame(absFrame);
        if(!other->anim_getKeyAtRelFrame(relFrame))
            other->anim_addKeyAtRelFrame(relFrame);
    }
}

void Animator::anim_deleteCurrentKey() {
    if(anim_mKeyOnCurrentFrame) anim_mKeyOnCurrentFrame->deleteKey();
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

int Animator::getInsertIdForKeyRelFrame(const int& relFrame) const {
    return getInsertIdForKeyRelFrame(relFrame, 0, anim_mKeys.count());
}

int Animator::getInsertIdForKeyRelFrame(
        const int& relFrame, const int& min, const int& max) const {
    if(min >= max) return min;
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

void Animator::anim_appendKey(const stdsptr<Key>& newKey) {
    if(!anim_mIsRecording) anim_mIsRecording = true;
    const int insertId = getInsertIdForKeyRelFrame(newKey->getRelFrame());
    anim_mKeys.insert(insertId, newKey);
    //anim_sortKeys();
    //mergeKeysIfNeeded();
    emit prp_addingKey(newKey.get());
    if(newKey->getRelFrame() == anim_mCurrentRelFrame)
        anim_setKeyOnCurrentFrame(newKey.get());
    anim_updateAfterChangedKey(newKey.get());
}

void Animator::anim_removeKey(const stdsptr<Key>& keyToRemove) {
    Key * const keyPtr = keyToRemove.get();

    int prevKeyRelFrame = anim_getPrevKeyRelFrame(keyPtr);
    if(prevKeyRelFrame != FrameRange::EMIN) prevKeyRelFrame++;
    int nextKeyRelFrame = anim_getNextKeyRelFrame(keyPtr);
    if(nextKeyRelFrame != FrameRange::EMAX) nextKeyRelFrame--;

    emit prp_removingKey(keyPtr);
    const int keyId = anim_getKeyIndex(keyPtr);
    anim_mKeys.removeAt(keyId);

    if(keyToRemove->getRelFrame() == anim_mCurrentRelFrame)
        anim_setKeyOnCurrentFrame(nullptr);

    prp_updateAfterChangedRelFrameRange({prevKeyRelFrame, nextKeyRelFrame});
}

void Animator::anim_moveKeyToRelFrame(Key *key, const int &newFrame) {
    const auto keySPtr = GetAsSPtr(key, Key);
    anim_removeKey(keySPtr);
    key->setRelFrame(newFrame);
    anim_appendKey(keySPtr);
}

void Animator::anim_updateKeyOnCurrrentFrame() {
    anim_setKeyOnCurrentFrame(anim_getKeyAtAbsFrame(anim_mCurrentAbsFrame));
}

DurationRectangleMovable *Animator::anim_getRectangleMovableAtPos(
        const int &relX, const int &minViewedFrame, const qreal &pixelsPerFrame) {
    Q_UNUSED(relX);
    Q_UNUSED(minViewedFrame);
    Q_UNUSED(pixelsPerFrame);
    return nullptr;
}

Key *Animator::anim_getKeyAtPos(const qreal &relX,
                               const int &minViewedFrame,
                               const qreal &pixelsPerFrame,
                               const int& keyRectSize) {
    const qreal relFrame = relX/pixelsPerFrame - 0.5 - prp_getFrameShift();
    const qreal absX = relX + minViewedFrame*pixelsPerFrame;
    const qreal absFrame = relFrame + minViewedFrame;
    qreal keySize = keyRectSize;
    if(SWT_isComplexAnimator()) {
        keySize *= 0.75;
    }
    if(pixelsPerFrame > keySize) {
        int relFrameInt = qRound(relFrame);
        const qreal distToFrame =
                qAbs((relFrameInt + 0.5)*pixelsPerFrame - relX);
        if(2*distToFrame > keySize) return nullptr;
    }
    //if(pressFrame < 0) pressFrame -= 1.;
    const qreal keyRectFramesSpan = 0.5*keySize/pixelsPerFrame;
    const int minPossibleKey = qFloor(absFrame - keyRectFramesSpan);
    const int maxPossibleKey = qCeil(absFrame + keyRectFramesSpan);
    Key* keyAtPos = nullptr;
    for(int i = maxPossibleKey; i >= minPossibleKey; i--) {
        const qreal distToFrame = qAbs((i + 0.5)*pixelsPerFrame - absX);
        if(2*distToFrame > keySize) continue;
        keyAtPos = anim_getKeyAtRelFrame(i);
        if(keyAtPos) return keyAtPos;
    }
    return nullptr;
}

void Animator::anim_addAllKeysToComplexAnimator(ComplexAnimator *target) {
    for(const auto &key : anim_mKeys) {
        target->ca_addDescendantsKey(key.get());
    }
}

void Animator::anim_removeAllKeysFromComplexAnimator(ComplexAnimator *target) {
    for(const auto &key : anim_mKeys) {
        target->ca_removeDescendantsKey(key.get());
    }
}

bool Animator::anim_hasKeys() const {
    return !anim_mKeys.isEmpty();
}

void Animator::anim_setRecordingWithoutChangingKeys(const bool &rec) {
    anim_setRecordingValue(rec);
}

void Animator::anim_setRecordingValue(const bool &rec) {
    if(rec == anim_mIsRecording) return;
    anim_mIsRecording = rec;
    emit anim_isRecordingChanged();
}

bool Animator::SWT_isAnimator() const { return true; }

bool Animator::anim_isRecording() {
    return anim_mIsRecording;
}

void Animator::anim_removeAllKeys() {
    if(anim_mKeys.isEmpty()) return;
    const auto keys = anim_mKeys;
    for(const auto& key : keys) anim_removeKey(key);
}

void Animator::anim_setKeyOnCurrentFrame(Key* const key) {
    anim_mKeyOnCurrentFrame = key;
    anim_afterKeyOnCurrentFrameChanged(key);
}

void Animator::anim_getKeysInRect(const QRectF &selectionRect,
                                 const qreal &pixelsPerFrame,
                                 QList<Key*> &keysList,
                                 const int& keyRectSize) {
    //selectionRect.translate(-getFrameShift(), 0.);
    int selLeftFrame = qRound(selectionRect.left());
    if(0.5*(pixelsPerFrame + keyRectSize) <
       selectionRect.left() - selLeftFrame*pixelsPerFrame) {
        selLeftFrame++;
    }
    int selRightFrame = qRound(selectionRect.right());
    if(0.5*(pixelsPerFrame - keyRectSize) >
       selectionRect.right() - selRightFrame*pixelsPerFrame) {
        selRightFrame--;
    }
    for(int i = selRightFrame; i >= selLeftFrame; i--) {
        Key * const keyAtPos = anim_getKeyAtAbsFrame(i);
        if(keyAtPos) keysList.append(keyAtPos);
    }
}

std::pair<int, int> Animator::anim_getPrevAndNextKeyIdForRelFrame(
        const int &frame) const {
    if(anim_mKeys.isEmpty()) return {-1, -1};
    const int lastKeyId = anim_mKeys.count() - 1;
    const int lastKeyRelFrame = anim_mKeys.last()->getRelFrame();
    if(frame > lastKeyRelFrame) {
        return {lastKeyId, lastKeyId + 1};
    } else if(frame == lastKeyRelFrame) {
        return {lastKeyId - 1, lastKeyId + 1};
    }
    const int firstKeyRelFrame = anim_mKeys.first()->getRelFrame();
    if(frame < firstKeyRelFrame) {
        return {-1, 0};
    } else if(frame == firstKeyRelFrame) {
        return {-1, 1};
    }
    int minId = 0;
    int maxId = lastKeyId;
    while(maxId - minId > 1) {
        const int guess = (maxId + minId)/2;
        const int keyFrame = anim_mKeys.at(guess)->getRelFrame();
        if(keyFrame > frame) {
            maxId = guess;
        } else if(keyFrame < frame) {
            minId = guess;
        } else {
            return { guess - 1, guess + 1 };
        }
    }
    return {minId, maxId};
}

std::pair<int, int> Animator::anim_getPrevAndNextKeyIdForRelFrameF(
        const qreal &frame) const {
    if(isInteger4Dec(frame))
        return anim_getPrevAndNextKeyIdForRelFrame(qRound(frame));
    const int fFrame = qFloor(frame);
    const int next = anim_getPrevAndNextKeyIdForRelFrame(fFrame).second;
    const int cFrame = qCeil(frame);
    const int prev = anim_getPrevAndNextKeyIdForRelFrame(cFrame).first;
    return {prev, next};
}

int Animator::anim_getCurrentAbsFrame() const {
    return anim_mCurrentAbsFrame;
}

int Animator::anim_getCurrentRelFrame() const {
    return anim_mCurrentRelFrame;
}

FrameRange Animator::prp_getIdenticalRelFrameRange(const int &relFrame) const {
    if(anim_mKeys.isEmpty()) return {FrameRange::EMIN, FrameRange::EMAX};
    const auto pn = anim_getPrevAndNextKeyIdForRelFrame(relFrame);
    const int prevId = pn.first;
    const int nextId = pn.second;

    Key *prevKey = anim_getKeyAtIndex(prevId);
    Key *nextKey = anim_getKeyAtIndex(nextId);
    const bool adjKeys = pn.second - pn.first == 1;
    Key * const keyAtRelFrame = adjKeys ? nullptr :
                                          anim_getKeyAtIndex(pn.first + 1);
    Key *prevPrevKey = keyAtRelFrame ? keyAtRelFrame : nextKey;
    Key *prevNextKey = keyAtRelFrame ? keyAtRelFrame : prevKey;

    int fId = relFrame;
    int lId = relFrame;

    while(true) {
        if(!prevKey) {
            fId = FrameRange::EMIN;
            break;
        }
        if(prevPrevKey) {
            if(prevKey->differsFromKey(prevPrevKey)) break;
        }
        fId = prevKey->getRelFrame();
        prevPrevKey = prevKey;
        prevKey = prevKey->getPrevKey();
    }

    while(true) {
        if(!nextKey) {
            lId = FrameRange::EMAX;
            break;
        }
        if(prevNextKey) {
            if(nextKey->differsFromKey(prevNextKey)) break;
        }
        lId = nextKey->getRelFrame();
        prevNextKey = nextKey;
        nextKey = nextKey->getNextKey();
    }

    return {fId, lId};
}

void Animator::anim_drawKey(QPainter * const p,
                            Key * const key,
                            const qreal &pixelsPerFrame,
                            const int &startFrame,
                            const int &rowHeight) {
    if(key->isSelected()) p->setBrush(Qt::yellow);
    else p->setBrush(Qt::red);
    if(key->isHovered()) p->setPen(QPen(Qt::black, 1.5));
    else p->setPen(QPen(Qt::black, 0.5));
    const qreal keyRadius = rowHeight * (SWT_isComplexAnimator() ? 0.21 : 0.3);
    const int frameRelToStart = key->getRelFrame() - startFrame;
    const QPointF keyCenter((frameRelToStart + 0.5)*pixelsPerFrame,
                            0.5*rowHeight);
    p->drawEllipse(keyCenter, keyRadius, keyRadius);
}

void Animator::drawTimelineControls(QPainter * const p,
                                    const qreal &pixelsPerFrame,
                                    const FrameRange &absFrameRange,
                                    const int &rowHeight) {
    p->save();
    p->translate(prp_getFrameShift()*pixelsPerFrame, 0);
    for(const auto &key : anim_mKeys) {
        if(absFrameRange.inRange(key->getAbsFrame())) {
            anim_drawKey(p, key.get(), pixelsPerFrame,
                         absFrameRange.fMin, rowHeight);
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
