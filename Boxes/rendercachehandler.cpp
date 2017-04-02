#include "rendercachehandler.h"
#include "boundingboxrendercontainer.h"
#include "key.h"
#include "Animators/complexanimator.h"

RenderCacheRangeChange::RenderCacheRangeChange(
        const RenderCacheRangeChangeType &typeT,
        const int &relFrameT) {
    type = typeT;
    relFrame = relFrameT;
    key = NULL;
}

RenderCacheRangeChange::RenderCacheRangeChange(
        const RenderCacheRangeChangeType &typeT,
        Key *keyToAdd) {
    type = typeT;
    relFrame = keyToAdd->getRelFrame();
    key = keyToAdd;
}

RenderCacheRange::RenderCacheRange(Key *minKey,
                                   Key *maxKey) {
    mMinKey = minKey;
    mMaxKey = maxKey;

    updateMinRelFrameFromKey();
    updateMaxRelFrameFromKey();

    updateAfterKeysChanged();
}

RenderCacheRange::RenderCacheRange(Key *minKey,
                                   Key *maxKey,
                                   const int &minFrame,
                                   const int &maxFrame) {
    mMinKey = minKey;
    mMaxKey = maxKey;

    mMinRelFrame = minFrame;
    mMaxRelFrame = maxFrame;
}

RenderCacheRange::~RenderCacheRange() {
    clearCache();
}

void RenderCacheRange::updateMinRelFrameFromKey() {
    if(mMinKey == NULL) {
        mMinRelFrame = INT_MIN;
    } else {
        mMinRelFrame = mMinKey->getRelFrame();
    }
}

void RenderCacheRange::updateMaxRelFrameFromKey() {
    if(mMaxKey == NULL) {
        mMaxRelFrame = INT_MAX ;
    } else {
        mMaxRelFrame = mMaxKey->getRelFrame();
    }
}

void RenderCacheRange::setInternalDifferencesPresent(const bool &bT) {
    if(bT == mInternalDifferences) return;
    mInternalDifferences = bT;
    clearCache();
}

bool RenderCacheRange::areInternalDifferencesPresent() const {
    return mInternalDifferences;
}

bool RenderCacheRange::relFrameInRange(const int &relFrame) {
    return relFrame >= getMinRelFrame() && relFrame < getMaxRelFrame();
}

bool RenderCacheRange::getRenderContainterIdAtRelFrame(const int &relFrame,
                                                       int *id) {
    int minId = 0;
    int maxId = mRenderContainers.count() - 1;

    while(minId < maxId) {
        int guess = (minId + maxId)/2;
        if(guess == maxId || guess == minId) return false;
        BoundingBoxRenderContainer *cont = mRenderContainers.at(guess);
        int contFrame = cont->getFrame();
        if(contFrame > relFrame) {
            maxId = guess;
        } else if(contFrame < relFrame) {
            minId = guess;
        } else {
            *id = guess;
            return true;
        }
    }
    return false;
}

int RenderCacheRange::getRenderContainterInsertIdAtRelFrame(
        const int &relFrame) {
    int minId = 0;
    int maxId = mRenderContainers.count() - 1;

    while(minId < maxId) {
        int guess = (minId + maxId)/2;
        if(guess == maxId || guess == minId) return maxId;
        BoundingBoxRenderContainer *cont = mRenderContainers.at(guess);
        int contFrame = cont->getFrame();
        if(contFrame > relFrame) {
            maxId = guess;
        } else if(contFrame < relFrame) {
            minId = guess;
        } else {
            return guess;
        }
    }
    return false;
}

BoundingBoxRenderContainer *RenderCacheRange::getRenderContainerAtRelFrame(
        const int &frame) {
    if(mInternalDifferences) {
        int id;
        if(getRenderContainterIdAtRelFrame(frame, &id)) {
            return mRenderContainers.at(id);
        }
        return NULL;
    } else {
        if(mRenderContainers.isEmpty()) {
            return NULL;
        }
        return mRenderContainers.first();
    }
}

BoundingBoxRenderContainer *
RenderCacheRange::createNewRenderContainerAtRelFrame(const int &frame) {
    BoundingBoxRenderContainer *cont = new BoundingBoxRenderContainer();
    cont->setFrame(frame);
    cont->incNumberPointers();
    if(mInternalDifferences) {
        int contId = getRenderContainterInsertIdAtRelFrame(frame);
        mRenderContainers.insert(contId, cont);
    } else {
        mRenderContainers.append(cont);
    }
    return cont;
}

void RenderCacheRange::clearCache() {
    foreach(BoundingBoxRenderContainer *cont, mRenderContainers) {
        cont->decNumberPointers();
    }

    mRenderContainers.clear();
}

const int &RenderCacheRange::getMaxRelFrame() {
    return mMaxRelFrame;
}

const int &RenderCacheRange::getMinRelFrame() {
    return mMinRelFrame;
}

void RenderCacheRange::updateAfterKeysChanged() {
    bool keysDiffer;
    if(mMinKey == NULL || mMaxKey == NULL) {
        keysDiffer = false;
    } else {
        keysDiffer = mMinKey->differsFromKey(mMaxKey);
    }
    setInternalDifferencesPresent(keysDiffer);
}

void RenderCacheRange::setMinKeyTMP(Key *minKey) {
    mMinKey = minKey;
}

void RenderCacheRange::setMaxKeyTMP(Key *maxKey) {
    mMaxKey = maxKey;
}

void RenderCacheRange::setMaxRelFrame(const int &maxRelFrame) {
    mMaxRelFrame = maxRelFrame;
}

void RenderCacheRange::setMinRelFrame(const int &minRelFrame) {
    mMinRelFrame = minRelFrame;
}

Key *RenderCacheRange::getMinKey() {
    return mMinKey;
}

Key *RenderCacheRange::getMaxKey() {
    return mMaxKey;
}

void RenderCacheRange::drawCacheOnTimeline(QPainter *p,
                                           const qreal &pixelsPerFrame,
                                           const qreal &drawY,
                                           const int &startFrame,
                                           const int &endFrame) {
    if(mInternalDifferences) {
        foreach(BoundingBoxRenderContainer *cont, mRenderContainers) {
            int dFrame = cont->getFrame() - startFrame;
            int xT = dFrame*pixelsPerFrame;
            p->fillRect(xT, drawY, pixelsPerFrame, 20, Qt::green);
            if(cont->getFrame() > endFrame) return;
        }
    } else {
        if(mRenderContainers.isEmpty()) return;
        int minDrawFrame = qMax(startFrame - 1, mMinRelFrame);
        int maxFrawFrame = qMin(endFrame + 2, mMaxRelFrame);
        int dFrame = minDrawFrame - startFrame;
        int xT = dFrame*pixelsPerFrame;
        p->fillRect(xT, drawY,
                    pixelsPerFrame*(maxFrawFrame - minDrawFrame),
                    20, Qt::green);
    }
}

RenderCacheHandler::RenderCacheHandler() {
    mRenderCacheRange << new RenderCacheRange(NULL, NULL);
}

void RenderCacheHandler::applyChanges() {
    foreach(const RenderCacheRangeChange &change, mRenderCacheRangeRemovals) {
        applyChange(change);
    }
    mRenderCacheRangeRemovals.clear();
    foreach(const RenderCacheRangeChange &change, mRenderCacheRangeAdds) {
        applyChange(change);
    }
    mRenderCacheRangeAdds.clear();
    foreach(const RenderCacheRangeChange &change, mRenderCacheRangeValueChanges) {
        applyChange(change);
    }
    mRenderCacheRangeValueChanges.clear();

    foreach(RenderCacheRange *rangeNeedingUpdate, mRangesNeedingUpdate) {
        rangeNeedingUpdate->updateAfterKeysChanged();
        rangeNeedingUpdate->clearCache();
    }
    mRangesNeedingUpdate.clear();

    //updateCurrentRenderContainerFromFrame();
}

void RenderCacheHandler::addRenderCacheRangeChange(
        const RenderCacheRangeChange &change) {
    if(change.type == ADD_KEY) {
        mRenderCacheRangeAdds << change;
    } else if(change.type == REMOVE_KEY) {
        mRenderCacheRangeRemovals << change;
    } else if(change.type == CHANGE_KEY) {
        mRenderCacheRangeValueChanges << change;
    }
}

BoundingBoxRenderContainer *RenderCacheHandler::getRenderContainerAtRelFrame(
                                const int &frame) {
    return getRenderCacheRangeContainingRelFrame(frame)->
            getRenderContainerAtRelFrame(frame);
}

BoundingBoxRenderContainer *RenderCacheHandler::createNewRenderContainerAtRelFrame(
                                const int &frame) {
    return getRenderCacheRangeContainingRelFrame(frame)->
            createNewRenderContainerAtRelFrame(frame);
}

void RenderCacheHandler::clearAllCache() {
    foreach(RenderCacheRange *range, mRenderCacheRange) {
        range->clearCache();
    }
}

void RenderCacheHandler::addRangeNeedingUpdate(const int &min,
                                               const int &max) {
    RenderCacheRange *first = getRenderCacheRangeContainingRelFrame(min);
    addRangeNeedingUpdate(first);
    int currId = mRenderCacheRange.indexOf(first) + 1;
    while(currId < mRenderCacheRange.count()) {
        RenderCacheRange *nextRange = mRenderCacheRange.at(currId);
        if(nextRange->getMinRelFrame() > max) return;
        addRangeNeedingUpdate(nextRange);
        currId++;
    }
}

void RenderCacheHandler::addAddedKey(Key *key) {
    addRenderCacheRangeChange(RenderCacheRangeChange(ADD_KEY, key));
}

void RenderCacheHandler::addRemovedKey(Key *key) {
    addRenderCacheRangeChange(RenderCacheRangeChange(REMOVE_KEY,
                                                     key->getRelFrame()));
}

void RenderCacheHandler::addChangedKey(Key *key) {
    addRenderCacheRangeChange(RenderCacheRangeChange(CHANGE_KEY,
                                                     key->getRelFrame()));
}

void RenderCacheHandler::updateCurrentRenderContainerFromFrame(
                                const int &relFrame) {
    BoundingBoxRenderContainer *contAtFrame =
            getRenderContainerAtRelFrame(relFrame);
    if(contAtFrame == NULL) {
        setCurrentRenderContainerVar(
                    createNewRenderContainerAtRelFrame(relFrame));
    } else {
        setCurrentRenderContainerVar(contAtFrame);
    }
}

bool RenderCacheHandler::updateCurrentRenderContainerFromFrameIfNotNull(
                                const int &relFrame) {
    BoundingBoxRenderContainer *cont =
            getRenderContainerAtRelFrame(relFrame);
    if(cont != NULL) {
        setCurrentRenderContainerVar(cont);
        return true;
    }
    return false;
}

void RenderCacheHandler::duplicateCurrentRenderContainerFrom(
                                BoundingBoxRenderContainer *cont) {
    mCurrentRenderContainer->duplicateFrom(cont);
}

void RenderCacheHandler::updateCurrentRenderContainerTransform(
                                const QMatrix &trans) {
    if(mCurrentRenderContainer == NULL) return;
    mCurrentRenderContainer->
            updatePaintTransformGivenNewCombinedTransform(trans);
}

void RenderCacheHandler::drawCurrentRenderContainer(QPainter *p) {
    if(mCurrentRenderContainer == NULL) return;
    mCurrentRenderContainer->draw(p);
}

void RenderCacheHandler::setCurrentRenderContainerVar(
                                BoundingBoxRenderContainer *cont) {
    if(mCurrentRenderContainer != NULL) {
        mCurrentRenderContainer->decNumberPointers();
    }
    mCurrentRenderContainer = cont;
    if(mCurrentRenderContainer != NULL) {
        mCurrentRenderContainer->incNumberPointers();
    }
}

void RenderCacheHandler::divideRenderCacheRangeAtRelFrame(
                                const int &divideRelFrame,
                                Key *newKey) {
    RenderCacheRange *oldRange =
            getRenderCacheRangeContainingRelFrame(divideRelFrame);
    divideRenderCacheRange(oldRange, newKey);
}

void RenderCacheHandler::divideRenderCacheRange(RenderCacheRange *oldRange,
                                                Key *newKey) {
    int oldId = mRenderCacheRange.indexOf(oldRange);
    Key *minKey = oldRange->getMinKey();
    Key *maxKey = oldRange->getMaxKey();
    removeRangeNeedingUpdate(oldRange);
    removeRenderCacheRangeFromList(oldRange);
    RenderCacheRange *range1 = new RenderCacheRange(minKey,
                                                    newKey,
                                                    oldRange->getMinRelFrame(),
                                                    newKey->getRelFrame());
    mRenderCacheRange.insert(oldId, range1);
    RenderCacheRange *range2 = new RenderCacheRange(newKey,
                                                    maxKey,
                                                    newKey->getRelFrame(),
                                                    oldRange->getMaxRelFrame());
    mRenderCacheRange.insert(oldId + 1, range2);
    delete oldRange;

    addRangeNeedingUpdate(range1);
    addRangeNeedingUpdate(range2);
}

void RenderCacheHandler::mergeRenderCacheRanges(RenderCacheRange *prevRange,
                                                RenderCacheRange *nextRange) {
    prevRange->setMaxRelFrame(nextRange->getMaxRelFrame());
    prevRange->setMaxKeyTMP(nextRange->getMinKey());
    removeRenderCacheRangeFromList(nextRange);
    removeRangeNeedingUpdate(nextRange);
    addRangeNeedingUpdate(prevRange);
    delete nextRange;
}

void RenderCacheHandler::getRenderCacheRangesWithBarrierAtRelFrame(
        RenderCacheRange **prevRange,
        RenderCacheRange **nextRange,
        const int &relFrame) {
    *prevRange = getRenderCacheRangeContainingRelFrame(relFrame - 1);
    *nextRange = getRenderCacheRangeContainingRelFrame(relFrame);
}

void RenderCacheHandler::removeRangesBarrierAtRelFrame(const int &relFrame) {
    RenderCacheRange *prevRange;
    RenderCacheRange *nextRange;
    getRenderCacheRangesWithBarrierAtRelFrame(&prevRange,
                                              &nextRange,
                                              relFrame);
    mergeRenderCacheRanges(prevRange, nextRange);
}

void RenderCacheHandler::updateAfterBarrierValueChanged(
    const int &barrierRelFrame) {
    if(mRenderCacheRange.count() == 1) {
        addRangeNeedingUpdate(mRenderCacheRange.first());
    } else {
        RenderCacheRange *prevRange;
        RenderCacheRange *nextRange;
        getRenderCacheRangesWithBarrierAtRelFrame(&prevRange,
                                                  &nextRange,
                                                  barrierRelFrame);
        addRangeNeedingUpdate(prevRange);
        addRangeNeedingUpdate(nextRange);
    }
}

RenderCacheRange *RenderCacheHandler::getRenderCacheRangeContainingRelFrame(
        const int &relFrame) {
    int minId = 0;
    int maxId = mRenderCacheRange.count() - 1;

    if(relFrame == INT_MIN) {
        return mRenderCacheRange.first();
    } else if(relFrame == INT_MAX) {
        return mRenderCacheRange.last();
    }

    while(minId < maxId) {
        int guess = (minId + maxId)/2;

        RenderCacheRange *guessRange = mRenderCacheRange.at(guess);
        if(guessRange->relFrameInRange(relFrame)) return guessRange;

        if(guessRange->getMinRelFrame() > relFrame) {
            if(maxId == guess) return mRenderCacheRange.at(minId);
            maxId = guess;
        } else {
            if(minId == guess) return mRenderCacheRange.at(maxId);
            minId = guess;
        }
    }
    return mRenderCacheRange.first();
}

void RenderCacheHandler::removeRenderCacheRangeFromList(
        RenderCacheRange *range) {
    mRenderCacheRange.removeOne(range);
}

void RenderCacheHandler::applyChange(const RenderCacheRangeChange &change) {
    if(change.type == REMOVE_KEY) {
        removeRangesBarrierAtRelFrame(change.relFrame);
    } else if(change.type == ADD_KEY) {
        divideRenderCacheRangeAtRelFrame(change.relFrame, change.key);
    } else if(change.type == CHANGE_KEY) {
        updateAfterBarrierValueChanged(change.relFrame);
    }
}

void RenderCacheHandler::removeRangeNeedingUpdate(RenderCacheRange *range) {
    mRangesNeedingUpdate.removeOne(range);
}

void RenderCacheHandler::addRangeNeedingUpdate(RenderCacheRange *range) {
    if(mRangesNeedingUpdate.contains(range)) return;
    mRangesNeedingUpdate << range;
}

void RenderCacheHandler::drawCacheOnTimeline(QPainter *p,
                           const qreal &pixelsPerFrame,
                           const qreal &drawY,
                           const int &startFrame,
                           const int &endFrame) {
    RenderCacheRange *first = getRenderCacheRangeContainingRelFrame(startFrame);
    first->drawCacheOnTimeline(p,
                               pixelsPerFrame,
                               drawY,
                               startFrame,
                               endFrame);
    int currId = mRenderCacheRange.indexOf(first) + 1;
    while(currId < mRenderCacheRange.count()) {
        RenderCacheRange *nextRange = mRenderCacheRange.at(currId);
        if(nextRange->getMinRelFrame() > endFrame) return;
        nextRange->drawCacheOnTimeline(p,
                                       pixelsPerFrame,
                                       drawY,
                                       startFrame,
                                       endFrame);
        currId++;
    }
}
