#include "rendercachehandler.h"
#include "boundingboxrendercontainer.h"
#include "key.h"
#include "Animators/complexanimator.h"
#include "durationrectangle.h"

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
                                   Key *maxKey,
                                   const int &minFrame,
                                   const int &maxFrame,
                                   RenderCacheHandler *parentHandler) {
    mMinKey = minKey;
    mMaxKey = maxKey;

    mMinRelFrame = minFrame;
    mMaxRelFrame = maxFrame;
    mParentHandler = parentHandler;
}

RenderCacheRange::~RenderCacheRange() {
    clearCache();
}

void RenderCacheRange::clearCacheAboveRelFrame(const int &min) {
    if(min >= mMaxRelFrame) return;
    mParentHandler->clearCacheForRelFrameRange(min, mMaxRelFrame);
}

void RenderCacheRange::clearCacheBelowRelFrame(const int &max) {
    if(mMinRelFrame >= max) return;
    mParentHandler->clearCacheForRelFrameRange(mMinRelFrame, max);
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

void RenderCacheRange::clearCache() {
    mParentHandler->clearCacheForRelFrameRange(mMinRelFrame, mMaxRelFrame);
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

void CacheHandler::removeRenderContainer(CacheContainer *cont) {
    mRenderContainers.removeOne(cont);
    cont->setParentCacheHandler(NULL);
    cont->decNumberPointers();
}

bool CacheHandler::getRenderContainterIdAtRelFrame(const int &relFrame,
                                                   int *id) {
    int minId = 0;
    int maxId = mRenderContainers.count() - 1;

    while(minId <= maxId) {
        int guess = (minId + maxId)/2;
        CacheContainer *cont = mRenderContainers.at(guess);
        if(cont->relFrameInRange(relFrame)) {
            *id = guess;
            return true;
        }
        int contFrame = cont->getMinRelFrame();
        if(contFrame > relFrame) {
            if(maxId == guess) {
                *id = minId;
                return mRenderContainers.at(minId)->relFrameInRange(relFrame);
            } else {
                maxId = guess;
            }
        } else if(contFrame < relFrame) {
            if(minId == guess) {
                *id = maxId;
                return mRenderContainers.at(maxId)->relFrameInRange(relFrame);
            } else {
                minId = guess;
            }
        } else {
            *id = guess;
            return true;
        }
    }
    return false;
}

int CacheHandler::getRenderContainterInsertIdAtRelFrame(const int &relFrame) {
    int minId = 0;
    int maxId = mRenderContainers.count();

    while(minId < maxId) {
        int guess = (minId + maxId)/2;
        CacheContainer *cont = mRenderContainers.at(guess);
        int contFrame = cont->getMinRelFrame();
        if(contFrame > relFrame) {
            if(guess == maxId) {
                return minId;
            }
            maxId = guess;
        } else if(contFrame < relFrame) {
            if(guess == minId) {
                return maxId;
            }
            minId = guess;
        }
    }
    return 0;
}

void CacheHandler::updateCurrentRenderContainerFromFrame(const int &relFrame) {
    CacheContainer *contAtFrame = getRenderContainerAtRelFrame(relFrame);
    if(contAtFrame == NULL) {
        setCurrentRenderContainerVar(
                    createNewRenderContainerAtRelFrame(relFrame));
    } else {
        setCurrentRenderContainerVar(contAtFrame);
    }
}

bool CacheHandler::updateCurrentRenderContainerFromFrameIfNotNull(
                                const int &relFrame) {
    CacheContainer *cont = getRenderContainerAtRelFrame(relFrame);
    if(cont != NULL) {
        setCurrentRenderContainerVar(cont);
        return true;
    }
    return false;
}

void CacheHandler::setCurrentRenderContainerVar(CacheContainer *cont) {
    if(cont == mCurrentRenderContainer) return;
    if(mCurrentRenderContainer != NULL) {
        mCurrentRenderContainer->decNumberPointers();
    }
    mCurrentRenderContainer = cont;
    if(mCurrentRenderContainer != NULL) {
        mCurrentRenderContainer->incNumberPointers();
        mCurrentRenderContainer->thisAccessed();
    }
}

CacheContainer *CacheHandler::getRenderContainerAtRelFrame(const int &frame) {
    int id;
    if(getRenderContainterIdAtRelFrame(frame, &id)) {
        return mRenderContainers.at(id);
    }
    return NULL;
}

RenderCacheHandler::RenderCacheHandler() {
    mRenderCacheRange << new RenderCacheRange(NULL, NULL,
                                              INT_MIN, INT_MAX,
                                              this);
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

void RenderCacheHandler::setNoCache(const bool &noCache) {
    mNoCache = noCache;
    clearCache();
    if(noCache) {
        RenderContainer *cont =
                new RenderContainer();
        mRenderContainers << cont;
        setCurrentRenderContainerVar(cont);
    }
}

void RenderCacheHandler::setupRenderRangeforAnimationRange() {
    if(mDurationRect == NULL) return;
    mDurationRect->connectRenderCacheHandler(this);
    if(mAnimationRenderCacheRange != NULL) {
        clearRenderRangeforAnimationRange();
    }

    if(mDurationRect->hasAnimationFrameRange()) {
        mAnimationRenderCacheRange = new AnimationRenderCacheRange(
                                        (AnimationRect*)mDurationRect,
                                        this);
    }
}

void RenderCacheHandler::clearRenderRangeforAnimationRange() {
    if(mAnimationRenderCacheRange != NULL) {
        applyChanges();

        delete mAnimationRenderCacheRange;

        mAnimationRenderCacheRange = NULL;
    }
}

void RenderCacheHandler::clearCache() {
    foreach(CacheContainer *cont, mRenderContainers) {
        cont->setParentCacheHandler(NULL);
        cont->decNumberPointers();
    }

    mRenderContainers.clear();
}

void RenderCacheHandler::clearCacheForRelFrameRange(const int &minFrame,
                                                    const int &maxFrame) {
    int minId = 0;
    getRenderContainterIdAtRelFrame(minFrame, &minId);
    int maxId = mRenderContainers.count() - 1;
    getRenderContainterIdAtRelFrame(maxFrame, &maxId);
    for(int i = minId; i <= maxId; i++) {
        CacheContainer *cont = mRenderContainers.takeAt(minId);
        cont->setParentCacheHandler(NULL);
        cont->decNumberPointers();
    }
    int minFrameT = minFrame;
    int maxFrameT = maxFrame;
    relRangeToAbsRange(&minFrameT, &maxFrameT);
    emit clearedCacheForAbsFrameRange(minFrameT, maxFrameT);
}

void RenderCacheHandler::addRangeNeedingUpdate(const int &min,
                                               const int &max) {
    RenderCacheRange *first = getRenderCacheRangeContainingRelFrame(min,
                                                                    false);
    addRangeNeedingUpdate(first);
    int currId = mRenderCacheRange.indexOf(first) + 1;
    while(currId < mRenderCacheRange.count()) {
        RenderCacheRange *nextRange = mRenderCacheRange.at(currId);
        if(nextRange->getMinRelFrame() >= max) return;
        addRangeNeedingUpdate(nextRange);
        currId++;
    }
    if(mAnimationRenderCacheRange != NULL) {
        if(mAnimationRenderCacheRange->getMinRelFrame() > max ||
           mAnimationRenderCacheRange->getMaxRelFrame() < min) return;
        addRangeNeedingUpdate(mAnimationRenderCacheRange);
    }
}

void RenderCacheHandler::addAddedKey(Key *key) {
    addRenderCacheRangeChange(RenderCacheRangeChange(ADD_KEY, key));
}

void RenderCacheHandler::addRemovedKey(Key *key) {
    addRenderCacheRangeChange(RenderCacheRangeChange(REMOVE_KEY,
                                                     key));
}

void RenderCacheHandler::addChangedKey(Key *key) {
    addRenderCacheRangeChange(RenderCacheRangeChange(CHANGE_KEY,
                                                     key->getRelFrame()));
}

void RenderCacheHandler::updateCurrentRenderContainerFromFrame(
                                const int &relFrame) {
    if(mNoCache) return;
    CacheHandler::updateCurrentRenderContainerFromFrame(relFrame);
}

bool RenderCacheHandler::updateCurrentRenderContainerFromFrameIfNotNull(
                                const int &relFrame) {
    if(mNoCache) return false;
    return CacheHandler::updateCurrentRenderContainerFromFrameIfNotNull(relFrame);
}

void RenderCacheHandler::duplicateCurrentRenderContainerFrom(
                            RenderContainer *cont) {
    ((RenderContainer*)mCurrentRenderContainer)->duplicateFrom(cont);
}

void RenderCacheHandler::updateCurrentRenderContainerTransform(
                                const QMatrix &trans) {
    if(mCurrentRenderContainer == NULL) return;
    ((RenderContainer*)mCurrentRenderContainer)->
            updatePaintTransformGivenNewCombinedTransform(trans);
}

void RenderCacheHandler::drawCurrentRenderContainer(QPainter *p) {
    if(mCurrentRenderContainer == NULL) return;
    ((RenderContainer*)mCurrentRenderContainer)->draw(p);
}

void RenderCacheHandler::divideRenderCacheRangeAtRelFrame(
                                const int &divideRelFrame,
                                Key *newKey) {
    RenderCacheRange *oldRange =
            getRenderCacheRangeContainingRelFrame(divideRelFrame, false);
    divideRenderCacheRange(oldRange, divideRelFrame, newKey);
}

void RenderCacheHandler::divideRenderCacheRange(RenderCacheRange *oldRange,
                                                const int &relFrame,
                                                Key *newKey) {
    int oldId = mRenderCacheRange.indexOf(oldRange);
    Key *minKey = oldRange->getMinKey();
    Key *maxKey = oldRange->getMaxKey();
    removeRangeNeedingUpdate(oldRange);
    removeRenderCacheRangeFromList(oldRange);
    RenderCacheRange *range1 = new RenderCacheRange(minKey,
                                                    newKey,
                                                    oldRange->getMinRelFrame(),
                                                    relFrame,
                                                    this);
    mRenderCacheRange.insert(oldId, range1);
    RenderCacheRange *range2 = new RenderCacheRange(newKey,
                                                    maxKey,
                                                    relFrame,
                                                    oldRange->getMaxRelFrame(),
                                                    this);
    mRenderCacheRange.insert(oldId + 1, range2);
    delete oldRange;

    addRangeNeedingUpdate(range1);
    addRangeNeedingUpdate(range2);
}

void RenderCacheHandler::mergeRenderCacheRanges(RenderCacheRange *prevRange,
                                                RenderCacheRange *nextRange) {
    prevRange->setMaxRelFrame(nextRange->getMaxRelFrame());
    prevRange->setMaxKeyTMP(nextRange->getMaxKey());
    removeRenderCacheRangeFromList(nextRange);
    removeRangeNeedingUpdate(nextRange);
    addRangeNeedingUpdate(prevRange);
    delete nextRange;
}

bool RenderCacheHandler::isThereBarrierAtRelFrame(const int &frame) {
    RenderCacheRange *prevRange;
    RenderCacheRange *nextRange;
    getRenderCacheRangesWithBarrierAtRelFrame(&prevRange,
                                              &nextRange,
                                              frame);
    return prevRange != nextRange;
}

void RenderCacheHandler::setDurationRectangle(DurationRectangle *durRect) {
    mDurationRect = durRect;
}

void RenderCacheHandler::insertRenderContainer(
                    RenderContainer *cont) {
    if(mRenderContainers.contains(cont)) return;
    cont->incNumberPointers();
    int contId = getRenderContainterInsertIdAtRelFrame(cont->getMinRelFrame());
    cont->setParentCacheHandler(this);
    mRenderContainers.insert(contId, cont);
}

#include <QDebug>
CacheContainer *RenderCacheHandler::createNewRenderContainerAtRelFrame(const int &frame) {
    RenderContainer *cont =
            new RenderContainer();
    cont->setParentCacheHandler(this);
    cont->setRelFrameRange(getMinRelFrameForContainerAtRel(frame),
                           getMaxRelFrameForContainerAtRel(frame));
    cont->incNumberPointers();
    int contId = getRenderContainterInsertIdAtRelFrame(frame);
    mRenderContainers.insert(contId, cont);
    return cont;
}

void RenderCacheHandler::updateAnimationRenderCacheRange() {
    if(mAnimationRenderCacheRange == NULL) return;
    AnimationRect *aRect = (AnimationRect*)mDurationRect;
    int durRectMin = aRect->getMinAnimationFrameAsRelFrame();
    int durRectMax = aRect->getMaxAnimationFrameAsRelFrame();
    mAnimationRenderCacheRange->clearCacheAboveRelFrame(durRectMax);
    mAnimationRenderCacheRange->clearCacheBelowRelFrame(durRectMin);
    mAnimationRenderCacheRange->setMinRelFrame(durRectMin);
    mAnimationRenderCacheRange->setMaxRelFrame(durRectMax);
}

void RenderCacheHandler::updateVisibilityRange() {
    if(mDurationRect == NULL) return;
    int newMinRelFrame = mDurationRect->getMinFrameAsRelFrame();
    int newMaxRelFrame = mDurationRect->getMaxFrameAsRelFrame() - 1;

    if(newMaxRelFrame < mMaxRelFrame) {
        for(int i = mRenderCacheRange.count() - 1; i >= 0; i--) {
            RenderCacheRange *range = mRenderCacheRange.at(i);
            range->clearCacheAboveRelFrame(newMaxRelFrame);
            if(range->getMinRelFrame() <= newMaxRelFrame) break;
        }
    }

    if(newMinRelFrame > mMinRelFrame) {
        foreach(RenderCacheRange *range, mRenderCacheRange) {
            range->clearCacheBelowRelFrame(newMinRelFrame);
            if(range->getMaxRelFrame() >= newMinRelFrame) break;
        }
    }

    if(mAnimationRenderCacheRange != NULL) {
        mAnimationRenderCacheRange->clearCacheAboveRelFrame(newMaxRelFrame);
        mAnimationRenderCacheRange->clearCacheBelowRelFrame(newMinRelFrame);
    }

    mMinRelFrame = newMinRelFrame;
    mMaxRelFrame = newMaxRelFrame;
}

void RenderCacheHandler::getRenderCacheRangesWithBarrierAtRelFrame(
        RenderCacheRange **prevRange,
        RenderCacheRange **nextRange,
        const int &relFrame) {
    *prevRange = getRenderCacheRangeContainingRelFrame(relFrame - 1, false);
    *nextRange = getRenderCacheRangeContainingRelFrame(relFrame, false);
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
                                        const int &relFrame,
                                        const bool &considerAnimationRange) {
    if(considerAnimationRange) {
        if(mAnimationRenderCacheRange != NULL) {
            if(mAnimationRenderCacheRange->relFrameInRange(relFrame)) {
                return mAnimationRenderCacheRange;
            }
        }
    }
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
    //qDebug() << range->getMinRelFrame() << range->getMaxRelFrame();
    mRangesNeedingUpdate << range;
}

bool RenderCacheHandler::areInternalDifferencesPresentFromAll(
                                    const int &relFrame) {
    RenderCacheRange *range = getRenderCacheRangeContainingRelFrame(relFrame);
    if(range->areInternalDifferencesPresent()) {
        return true;
    }
    int absFrame = relFrameToAbsFrame(relFrame);
    foreach(RenderCacheHandler *handler, mInfluencingHandlers) {
        if(handler->areInternalDifferencesPresentFromAll(
                    handler->absFrameToRelFrame(absFrame))) {
            return true;
        }
    }

    return false;
}

void RenderCacheHandler::addInfluencingHandler(RenderCacheHandler *handler) {
    mInfluencingHandlers << handler;
}

void RenderCacheHandler::removeInfluencingHandler(RenderCacheHandler *handler) {
    mInfluencingHandlers.removeOne(handler);
}
#include "boundingbox.h"
int RenderCacheHandler::relFrameToAbsFrame(const int &relFrame) {
    return mParentBox->prp_relFrameToAbsFrame(relFrame);
}

int RenderCacheHandler::absFrameToRelFrame(const int &absFrame) {
    return mParentBox->prp_absFrameToRelFrame(absFrame);
}

void RenderCacheHandler::clearCacheForAbsFrameRange(const int &minFrame,
                                                    const int &maxFrame) {
    int minFrameT = minFrame;
    int maxFrameT = maxFrame;
    absRangeToRelRange(&minFrameT, &maxFrameT);
    clearCacheForRelFrameRange(minFrameT, maxFrameT);
}

int RenderCacheHandler::getMaxRelFrameForContainerAtRel(
                                const int &relFrame) {
    int maxRelFrame;
    RenderCacheRange *range = getRenderCacheRangeContainingRelFrame(relFrame);
    if(range->areInternalDifferencesPresent()) {
        return relFrame + 1;
    } else {
        maxRelFrame = range->getMaxRelFrame();
    }
    int absFrame = relFrameToAbsFrame(relFrame);
    foreach(RenderCacheHandler *handler, mInfluencingHandlers) {
        if(handler->areInternalDifferencesPresentFromAll(
                    handler->absFrameToRelFrame(absFrame))) {
            return relFrame + 1;
        } else {
            int maxRelFrameT = absFrameToRelFrame(
                        handler->relFrameToAbsFrame(
                        handler->getMaxRelFrameForContainerAtRel(
                            handler->absFrameToRelFrame(absFrame)) ));
            if(maxRelFrame > maxRelFrameT) {
                maxRelFrame = maxRelFrameT;
            }
        }
    }
    if(relFrame == maxRelFrame) return relFrame + 1;
    return maxRelFrame;
}

int RenderCacheHandler::getMinRelFrameForContainerAtRel(
                                const int &relFrame) {
    int minRelFrame;
    RenderCacheRange *range = getRenderCacheRangeContainingRelFrame(relFrame);
    if(range->areInternalDifferencesPresent()) {
        return relFrame;
    } else {
        minRelFrame = range->getMinRelFrame();
    }
    int absFrame = relFrameToAbsFrame(relFrame);
    foreach(RenderCacheHandler *handler, mInfluencingHandlers) {
        if(handler->areInternalDifferencesPresentFromAll(
                    handler->absFrameToRelFrame(absFrame)) ) {
            return relFrame;
        } else {
            int minRelFrameT = absFrameToRelFrame(
                        handler->relFrameToAbsFrame(
                        handler->getMinRelFrameForContainerAtRel(
                            handler->absFrameToRelFrame(absFrame)) ));
            if(minRelFrame < minRelFrameT) {
                minRelFrame = minRelFrameT;
            }
        }
    }
    return minRelFrame;
}

void RenderCacheHandler::drawCacheOnTimeline(QPainter *p,
                           const qreal &pixelsPerFrame,
                           const qreal &drawY,
                           const int &startFrame,
                           const int &endFrame) {
    p->setBrush(QColor(0, 255, 0, 75));
    p->setPen(Qt::NoPen);
    int lastDrawnFrame = startFrame;
    int lastDrawX = 0;
    foreach(CacheContainer *cont, mRenderContainers) {
        int maxFrame = cont->getMaxRelFrame();
        int minFrame = cont->getMinRelFrame();
        if(maxFrame < startFrame) continue;
        if(minFrame > endFrame + 1) return;

        if(maxFrame > endFrame) maxFrame = endFrame + 1;
        if(minFrame < startFrame) minFrame = startFrame;

        int dFrame = minFrame - startFrame;
        int xT = dFrame*pixelsPerFrame;

        int widthT = pixelsPerFrame*(maxFrame - minFrame);
        if(lastDrawnFrame == minFrame) {
            widthT += xT - lastDrawX;
            xT = lastDrawX;
        }
        p->drawRect(xT, drawY, widthT, 20);
        lastDrawnFrame = maxFrame;
        lastDrawX = xT + widthT;
    }
}

AnimationRenderCacheRange::AnimationRenderCacheRange(
        AnimationRect *durationRect, RenderCacheHandler *parentHandler) :
    RenderCacheRange(NULL, NULL,
                     durationRect->getMinAnimationFrameAsRelFrame(),
                     durationRect->getMaxAnimationFrameAsRelFrame(),
                     parentHandler) {
    mDurationRect = durationRect;
    mInternalDifferences = true;
}

void AnimationRenderCacheRange::addInternalKey(Key *key) {
    mInternalKeys.insert(getKeyInsertIdForFrame(key->getRelFrame()), key);
    updateMaxKey();
    updateMinKey();
}

void AnimationRenderCacheRange::removeInternalKey(Key *key) {
    mInternalKeys.removeOne(key);
    updateMaxKey();
    updateMinKey();
}

int AnimationRenderCacheRange::getKeyInsertIdForFrame(const int &frame) {
    int minId = 0;
    int maxId = mInternalKeys.count();

    while(minId < maxId) {
        int guess = (minId + maxId)/2;
        Key *guessKey = mInternalKeys.at(guess);
        int guessFrame = guessKey->getRelFrame();
        if(guessFrame > frame) {
            if(guess == maxId) {
                return minId;
            }
            maxId = guess;
        } else if(guessFrame < frame) {
            if(guess == minId) {
                return maxId;
            }
            minId = guess;
        }
    }
    return 0;
}

void AnimationRenderCacheRange::addAllInternalKeysToHandler(
                                        RenderCacheHandler *handler) {
    foreach(Key *key, mInternalKeys) {
        handler->divideRenderCacheRangeAtRelFrame(key->getRelFrame(), key);
    }
}
