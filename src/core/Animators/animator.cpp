#include "animator.h"

#include <QPainter>
#include "complexanimator.h"
#include "key.h"
#include "fakecomplexanimator.h"
#include "PropertyUpdaters/propertyupdater.h"
#include "qrealpoint.h"
#include "simplemath.h"

Animator::Animator(const QString& name) : Property(name), anim_mKeys(this) {}

void Animator::anim_scaleTime(const int pivotAbsFrame, const qreal scale) {
    for(const auto &key : anim_mKeys) {
        key->scaleFrameAndUpdateParentAnimator(pivotAbsFrame, scale, false);
    }
}

void Animator::anim_shiftAllKeys(const int shift) {
    for(const auto &key : anim_mKeys) {
        anim_moveKeyToRelFrame(key, key->getRelFrame() + shift);
    }
}

bool Animator::anim_nextRelFrameWithKey(const int relFrame,
                                        int &nextRelFrame) {
    const auto key = anim_getNextKey(relFrame);
    if(!key) return false;
    nextRelFrame = key->getRelFrame();
    return true;
}

bool Animator::anim_prevRelFrameWithKey(const int relFrame,
                                       int &prevRelFrame) {
    const auto key = anim_getPrevKey(relFrame);
    if(!key) return false;
    prevRelFrame = key->getRelFrame();
    return true;
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
    mFakeComplexAnimator = enve::make_shared<FakeComplexAnimator>(prp_mName, this);
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

int Animator::anim_getPrevKeyRelFrame(const int relFrame) const {
    Key * const prevKey = anim_getPrevKey(relFrame);
    if(!prevKey) return FrameRange::EMIN;
    return prevKey->getRelFrame();
}

int Animator::anim_getNextKeyRelFrame(const int relFrame) const {
    Key * const nextKey = anim_getNextKey(relFrame);
    if(!nextKey) return FrameRange::EMAX;
    return nextKey->getRelFrame();
}

void Animator::prp_afterChangedAbsRange(const FrameRange &range) {
    if(range.inRange(anim_mCurrentAbsFrame)) prp_callUpdater();
    emit prp_absFrameRangeChanged(range);
}

void Animator::anim_updateAfterChangedKey(Key * const key) {
    if(SWT_isComplexAnimator()) return;
    if(!key) {
        prp_afterWholeInfluenceRangeChanged();
        return;
    }
    int prevKeyRelFrame = anim_getPrevKeyRelFrame(key);
    if(prevKeyRelFrame != FrameRange::EMIN) prevKeyRelFrame++;
    int nextKeyRelFrame = anim_getNextKeyRelFrame(key);
    if(nextKeyRelFrame != FrameRange::EMAX) nextKeyRelFrame--;
    prp_afterChangedRelRange({prevKeyRelFrame, nextKeyRelFrame});
}

void Animator::anim_setAbsFrame(const int frame) {
    anim_mCurrentAbsFrame = frame;
    anim_updateRelFrame();
    anim_updateKeyOnCurrrentFrame();
    //anim_callFrameChangeUpdater();
}

void Animator::anim_updateRelFrame() {
    anim_mCurrentRelFrame = anim_mCurrentAbsFrame - prp_getFrameShift();
}

void Animator::anim_setRecording(const bool rec) {
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

struct KeyPair {
    KeyPair(Key * const key1, Key * const key2) :
        fKey1(key1), fKey2(key2) {}

    Key * fKey1;
    Key * fKey2;
};

void Animator::anim_mergeKeysIfNeeded() {
    anim_mKeys.mergeAll();
}

bool Animator::anim_getClosestsKeyOccupiedRelFrame(const int frame,
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

int Animator::anim_getPrevKeyId(const int relFrame) const {
    return anim_getPrevAndNextKeyId(relFrame).first;
}

int Animator::anim_getNextKeyId(const int relFrame) const {
    return anim_getPrevAndNextKeyId(relFrame).second;
}


int Animator::anim_getKeyIndex(const Key * const key) const {
    int index = -1;
    for(int i = 0; i < anim_mKeys.count(); i++) {
        if(anim_mKeys.atId(i) == key) index = i;
    }
    return index;
}

void Animator::anim_addKeysWhereOtherHasKeys(const Animator * const other) {
    for(const auto& otherKey : other->anim_mKeys) {
        const int absFrame = otherKey->getAbsFrame();
        const int relFrame = prp_absFrameToRelFrame(absFrame);
        if(!anim_getKeyAtRelFrame(relFrame))
            anim_addKeyAtRelFrame(relFrame);
    }
}
#include "basicreadwrite.h"
void Animator::readKeys(QIODevice * const src) {
    int nKeys;
    src->read(rcChar(&nKeys), sizeof(int));
    if(nKeys < 0 || nKeys > 10000)
        RuntimeThrow("Invalid key count " + std::to_string(nKeys));
    for(int i = 0; i < nKeys; i++) {
        const auto key = createKey();
        key->readKey(src);
        anim_appendKey(key);
    }
}

void Animator::writeSelectedKeys(QIODevice * const dst) {
    const int nKeys = anim_mSelectedKeys.count();
    dst->write(rcConstChar(&nKeys), sizeof(int));
    for(const auto& key : anim_mSelectedKeys)
        key->writeKey(dst);
}

void Animator::writeKeys(QIODevice *target) const {
    const int nKeys = anim_mKeys.count();
    target->write(rcConstChar(&nKeys), sizeof(int));
    for(const auto &key : anim_mKeys) {
        key->writeKey(target);
    }
}

void Animator::anim_coordinateKeysWith(Animator * const other) {
    anim_addKeysWhereOtherHasKeys(other);
    other->anim_addKeysWhereOtherHasKeys(this);
}

void Animator::anim_deleteCurrentKey() {
    if(anim_mKeyOnCurrentFrame) anim_mKeyOnCurrentFrame->deleteKey();
}

void Animator::anim_callFrameChangeUpdater() {
    if(prp_mUpdater) prp_mUpdater->frameChangeUpdate();
}

void Animator::anim_updateAfterShifted() {
    for(const auto &key : anim_mKeys) {
        emit prp_removingKey(key);
        emit prp_addingKey(key);
    }
}

void Animator::anim_appendKey(const stdsptr<Key>& newKey) {
    const bool isComplex = SWT_isComplexAnimator();
    if(!isComplex) anim_setRecordingValue(true);
    anim_mKeys.add(newKey);
    emit prp_addingKey(newKey.get());
    if(newKey->getRelFrame() == anim_mCurrentRelFrame)
        anim_setKeyOnCurrentFrame(newKey.get());
    if(!isComplex) anim_updateAfterChangedKey(newKey.get());
}

void Animator::anim_removeKey(const stdsptr<Key>& keyToRemove) {
    removeKeyFromSelected(keyToRemove.get());
    removeKeyWithoutDeselecting(keyToRemove);
}

void Animator::removeKeyWithoutDeselecting(const stdsptr<Key>& keyToRemove) {
    const bool isComplex = SWT_isComplexAnimator();
    Key * const keyPtr = keyToRemove.get();
    anim_mKeys.remove(keyToRemove);

    const int rFrame = keyPtr->getRelFrame();

    emit prp_removingKey(keyPtr);
    if(rFrame == anim_mCurrentRelFrame)
        anim_setKeyOnCurrentFrame(nullptr);

    if(isComplex) return;
    const int prevKeyRelFrame = anim_getPrevKeyRelFrame(rFrame);
    const int nextKeyRelFrame = anim_getNextKeyRelFrame(rFrame);
    const int affectedMin = prevKeyRelFrame == FrameRange::EMIN ?
                FrameRange::EMIN :
                qMin(prevKeyRelFrame + 1, rFrame);
    const int affectedMax = nextKeyRelFrame == FrameRange::EMAX ?
                FrameRange::EMAX :
                qMax(nextKeyRelFrame - 1, rFrame);
    prp_afterChangedRelRange({affectedMin, affectedMax});
}

void Animator::anim_moveKeyToRelFrame(Key * const key, const int newFrame) {
    const auto keySPtr = key->ref<Key>();
    removeKeyWithoutDeselecting(keySPtr);
    key->setRelFrame(newFrame);
    anim_appendKey(keySPtr);
}

void Animator::anim_updateKeyOnCurrrentFrame() {
    const auto key = anim_getKeyAtAbsFrame(anim_mCurrentAbsFrame);
    anim_setKeyOnCurrentFrame(key);
}

DurationRectangleMovable *Animator::anim_getTimelineMovable(
        const int relX, const int minViewedFrame, const qreal pixelsPerFrame) {
    Q_UNUSED(relX);
    Q_UNUSED(minViewedFrame);
    Q_UNUSED(pixelsPerFrame);
    return nullptr;
}

Key *Animator::anim_getKeyAtPos(const qreal relX,
                               const int minViewedFrame,
                               const qreal pixelsPerFrame,
                               const int keyRectSize) {
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
        keyAtPos = anim_getKeyAtAbsFrame(i);
        if(keyAtPos) return keyAtPos;
    }
    return nullptr;
}

void Animator::anim_addAllKeysToComplexAnimator(ComplexAnimator *target) {
    for(const auto &key : anim_mKeys) {
        target->ca_addDescendantsKey(key);
    }
}

void Animator::anim_removeAllKeysFromComplexAnimator(ComplexAnimator *target) {
    for(const auto &key : anim_mKeys) {
        target->ca_removeDescendantsKey(key);
    }
}

bool Animator::anim_hasKeys() const {
    return !anim_mKeys.isEmpty();
}

void Animator::anim_setRecordingWithoutChangingKeys(const bool rec) {
    anim_setRecordingValue(rec);
}

void Animator::anim_setRecordingValue(const bool rec) {
    if(rec == anim_mIsRecording) return;
    anim_mIsRecording = rec;
    emit anim_isRecordingChanged();
}

bool Animator::anim_isRecording() {
    return anim_mIsRecording;
}

void Animator::anim_removeAllKeys() {
    if(anim_mKeys.isEmpty()) return;
    const auto keys = anim_mKeys;
    for(const auto& key : keys) anim_removeKey(key->ref<Key>());
}

void Animator::anim_setKeyOnCurrentFrame(Key* const key) {
    anim_mKeyOnCurrentFrame = key;
    anim_afterKeyOnCurrentFrameChanged(key);
}

void Animator::anim_getKeysInRect(const QRectF &selectionRect,
                                 const qreal pixelsPerFrame,
                                 QList<Key*> &keysList,
                                 const int keyRectSize) {
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

std::pair<int, int> Animator::anim_getPrevAndNextKeyId(const int relFrame) const {
    return anim_mKeys.prevAndNextId(relFrame);
}

std::pair<int, int> Animator::anim_getPrevAndNextKeyIdF(const qreal relFrame) const {
    if(isInteger4Dec(relFrame))
        return anim_getPrevAndNextKeyId(qRound(relFrame));
    const int fFrame = qFloor(relFrame);
    const int next = anim_getNextKeyId(fFrame);
    const int cFrame = qCeil(relFrame);
    const int prev = anim_getPrevKeyId(cFrame);
    return {prev, next};
}

int Animator::anim_getCurrentAbsFrame() const {
    return anim_mCurrentAbsFrame;
}

int Animator::anim_getCurrentRelFrame() const {
    return anim_mCurrentRelFrame;
}

FrameRange Animator::prp_getIdenticalRelRange(const int relFrame) const {
    if(anim_mKeys.isEmpty()) return {FrameRange::EMIN, FrameRange::EMAX};
    const auto pn = anim_getPrevAndNextKeyId(relFrame);
    const int prevId = pn.first;
    const int nextId = pn.second;

    Key *prevKey = anim_getKeyAtIndex(prevId);
    Key *nextKey = anim_getKeyAtIndex(nextId);
    const bool adjKeys = nextId - prevId == 1;
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
                            const qreal pixelsPerFrame,
                            const int startFrame,
                            const int rowHeight) {
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
                                    const qreal pixelsPerFrame,
                                    const FrameRange &absFrameRange,
                                    const int rowHeight) {
    p->translate(prp_getFrameShift()*pixelsPerFrame, 0);
    const auto relRange = prp_absRangeToRelRange(absFrameRange);
    const auto idRange = frameRangeToKeyIdRange(relRange);
    for(int i = idRange.fMin; i <= idRange.fMax; i++) {
        if(i < 0 || i >= anim_mKeys.count()) continue;
        const auto& key = anim_mKeys.atId(i);
        anim_drawKey(p, key, pixelsPerFrame,
                     absFrameRange.fMin, rowHeight);
    }
}

#include "typemenu.h"
void Animator::setupTreeViewMenu(PropertyMenu * const menu) {
    Property::setupTreeViewMenu(menu);
    menu->addSeparator();
    const PropertyMenu::PlainSelectedOp<Animator> aOp =
    [](Animator * animTarget) {
        animTarget->anim_saveCurrentValueAsKey();
    };
    menu->addPlainAction("Add Key", aOp)->setDisabled(anim_getKeyOnCurrentFrame());

    const PropertyMenu::PlainSelectedOp<Animator> dOp =
    [](Animator * animTarget) {
        animTarget->anim_deleteCurrentKey();
    };
    menu->addPlainAction("Delete Key", dOp)->setEnabled(anim_getKeyOnCurrentFrame());
}

bool Animator::hasSelectedKeys() const {
    return !anim_mSelectedKeys.isEmpty();
}

void Animator::addKeyToSelected(Key * const key) {
    if(key->isSelected()) return;
    anim_mSelectedKeys << key;
    key->setSelected(true);
}

void Animator::removeKeyFromSelected(Key * const key) {
    if(key->isSelected()) {
        key->setSelected(false);
        anim_mSelectedKeys.removeOne(key);
    }
}

void Animator::deselectAllKeys() {
    for(const auto& key : anim_mSelectedKeys) {
        key->setSelected(false);
    }
    anim_mSelectedKeys.clear();
}

void Animator::selectAllKeys() {
    for(const auto& key : anim_mKeys) {
        addKeyToSelected(key);
    }
}

void Animator::incSelectedKeysFrame(const int dFrame) {
    for(const auto& key : anim_mSelectedKeys) {
        const int newFrame = key->getRelFrame() + dFrame;
        anim_moveKeyToRelFrame(key, newFrame);
    }
}

void Animator::scaleSelectedKeysFrame(const int absPivotFrame,
                                      const qreal scale) {
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
    anim_mergeKeysIfNeeded();
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

void OverlappingKeys::merge() {
    if(mKeys.count() < 2) return;
    Key * target = nullptr;
    for(const auto& iKey : mKeys) {
        if(iKey->isDescendantSelected()) {
            target = iKey.get();
            break;
        }
    }
    if(!target) target = mKeys.last().get();

    if(mAnimator->SWT_isComplexAnimator()) {
        const auto cTarget = static_cast<ComplexKey*>(target);
        for(int i = 0; i < mKeys.count(); i++) {
            const auto& iKey = mKeys.at(i);
            if(iKey.get() == target) continue;
            const auto cKey = iKey->ref<ComplexKey>();
            cKey->moveAllKeysTo(cTarget);
            mAnimator->anim_removeKey(iKey);
            i--;
        }
    } else {
        for(int i = 0; i < mKeys.count(); i++) {
            const auto& iKey = mKeys.at(i);
            if(iKey.get() == target) continue;
            mAnimator->anim_removeKey(iKey);
            i--;
        }
    }
}

void OverlappingKeyList::add(const stdsptr<Key> &key) {
    const int relFrame = key->getRelFrame();
    const auto notLess = lowerBound(relFrame);
    if(notLess == mList.end()) {
        mList.append(OverlappingKeys(key, mAnimator));
    } else {
        if(notLess->getFrame() == relFrame) notLess->addKey(key);
        else {
            const int insertId = notLess - mList.begin();
            mList.insert(insertId, OverlappingKeys(key, mAnimator));
        }
    }
}

bool OKeyFrameMore(const int relFrame, const OverlappingKeys& keys) {
    return keys.getFrame() > relFrame;
}

OverlappingKeyList::OKeyListCIter
    OverlappingKeyList::upperBound(const int relFrame) const {
    return std::upper_bound(mList.begin(), mList.end(),
                            relFrame, OKeyFrameMore);
}

OverlappingKeyList::OKeyListIter
    OverlappingKeyList::upperBound(const int relFrame) {
    return std::upper_bound(mList.begin(), mList.end(),
                            relFrame, OKeyFrameMore);
}

bool OKeyFrameLess(const OverlappingKeys& keys,
                   const int relFrame) {
    return keys.getFrame() < relFrame;
}

OverlappingKeyList::OKeyListCIter
    OverlappingKeyList::lowerBound(const int relFrame) const {
    return std::lower_bound(mList.begin(), mList.end(),
                            relFrame, OKeyFrameLess);
}

OverlappingKeyList::OKeyListIter
    OverlappingKeyList::lowerBound(const int relFrame) {
    return std::lower_bound(mList.begin(), mList.end(),
                            relFrame, OKeyFrameLess);
}

int OverlappingKeyList::idAtFrame(const int relFrame) const {
    const auto notPrevious = lowerBound(relFrame);
    if(notPrevious == mList.end()) return -1;
    if(notPrevious->getFrame() == relFrame)
        return notPrevious - mList.begin();
    return -1;
}
