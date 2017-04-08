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
                                   const int &maxFrame) {
    mMinKey = minKey;
    mMaxKey = maxKey;

    mMinRelFrame = minFrame;
    mMaxRelFrame = maxFrame;
}

RenderCacheRange::~RenderCacheRange() {
    clearCache();
}

void RenderCacheRange::clearCacheAboveRelFrame(const int &max) {
    for(int i = mRenderContainers.count() - 1; i >= 0; i--) {
        CacheBoundingBoxRenderContainer *cont = mRenderContainers.at(i);
        if(cont->getFrame() <= max) return;
        cont->freeThis();
    }
}

void RenderCacheRange::clearCacheBelowRelFrame(const int &min) {
    for(int i = 0; i < mRenderContainers.count(); i++) {
        CacheBoundingBoxRenderContainer *cont = mRenderContainers.at(i);
        if(cont->getFrame() >= min) return;
        cont->freeThis();
        i--;
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

    while(minId <= maxId) {
        int guess = (minId + maxId)/2;
        CacheBoundingBoxRenderContainer *cont = mRenderContainers.at(guess);
        int contFrame = cont->getFrame();
        if(contFrame > relFrame) {
            if(maxId == guess) {
                *id = minId;
                return mRenderContainers.at(minId)->getFrame() == relFrame;
            } else {
                maxId = guess;
            }
        } else if(contFrame < relFrame) {
            if(minId == guess) {
                *id = maxId;
                return mRenderContainers.at(maxId)->getFrame() == relFrame;
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

int RenderCacheRange::getRenderContainterInsertIdAtRelFrame(
        const int &relFrame) {
    int minId = 0;
    int maxId = mRenderContainers.count();

    while(minId < maxId) {
        int guess = (minId + maxId)/2;
        CacheBoundingBoxRenderContainer *cont = mRenderContainers.at(guess);
        int contFrame = cont->getFrame();
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

void RenderCacheRange::insertRenderContainer(
                    CacheBoundingBoxRenderContainer *cont) {
    if(mRenderContainers.contains(cont)) return;
    int contId = getRenderContainterInsertIdAtRelFrame(cont->getFrame());
    cont->setParentRagne(this);
    mRenderContainers.insert(contId, cont);
    cont->incNumberPointers();
}

void RenderCacheRange::removeRenderContainer(
        CacheBoundingBoxRenderContainer *cont) {
    mRenderContainers.removeOne(cont);
    cont->decNumberPointers();
}

CacheBoundingBoxRenderContainer *
RenderCacheRange::getRenderContainerAtRelFrame(
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
#include <QDebug>
CacheBoundingBoxRenderContainer *
RenderCacheRange::createNewRenderContainerAtRelFrame(const int &frame) {
    CacheBoundingBoxRenderContainer *cont =
            new CacheBoundingBoxRenderContainer();
    cont->setParentRagne(this);
    cont->setRelFrame(frame);
    cont->incNumberPointers();
    if(mInternalDifferences) {
        int contId = getRenderContainterInsertIdAtRelFrame(frame);
        mRenderContainers.insert(contId, cont);
        qDebug() << "frame: " << frame << " at: " << contId;
    } else {
        mRenderContainers.append(cont);
    }
    return cont;
}

void RenderCacheRange::clearCache() {
    foreach(CacheBoundingBoxRenderContainer *cont, mRenderContainers) {
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
                                           const int &endFrame,
                                           int *lastDrawnRelToStartFrameP,
                                           int *lastDrawFrameRightPosP) {
    int lastDrawnRelToStartFrame = *lastDrawnRelToStartFrameP;
    int lastDrawFrameRightPos = *lastDrawFrameRightPosP;
    if(mInternalDifferences) {
        foreach(CacheBoundingBoxRenderContainer *cont, mRenderContainers) {
            int dFrame = cont->getFrame() - startFrame;
            int xT = dFrame*pixelsPerFrame;
            int widthT = pixelsPerFrame;
            if(lastDrawnRelToStartFrame == dFrame -  1) {
                widthT += xT - lastDrawFrameRightPos;
                xT = lastDrawFrameRightPos;
            }
            p->drawRect(xT, drawY, widthT, 20);
            lastDrawnRelToStartFrame = dFrame;
            lastDrawFrameRightPos = xT + widthT;
            if(cont->getFrame() > endFrame) return;
        }
    } else {
        if(mRenderContainers.isEmpty()) return;
        int minDrawFrame = qMax(startFrame, mMinRelFrame);
        int maxFrawFrame = qMin(endFrame + 1, mMaxRelFrame);
        int dFrame = minDrawFrame - startFrame;
        int xT = dFrame*pixelsPerFrame;
        int widthT = pixelsPerFrame*(maxFrawFrame - minDrawFrame);
        if(lastDrawnRelToStartFrame == dFrame -  1) {
            widthT += xT - lastDrawFrameRightPos;
            xT = lastDrawFrameRightPos;
        }
        p->drawRect(xT, drawY,
                    widthT,
                    20);
        lastDrawnRelToStartFrame = dFrame;
        lastDrawFrameRightPos = xT + widthT;
    }
    *lastDrawnRelToStartFrameP = lastDrawnRelToStartFrame;
    *lastDrawFrameRightPosP = lastDrawFrameRightPos;
}

RenderCacheHandler::RenderCacheHandler() {
    mRenderCacheRange << new RenderCacheRange(NULL, NULL,
                                              INT_MIN, INT_MAX);
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

    if(mCurrentRenderContainer == NULL) return;
    getRenderCacheRangeContainingRelFrame(
        mCurrentRenderContainer->getFrame())->
            insertRenderContainer(mCurrentRenderContainer);
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

CacheBoundingBoxRenderContainer *
RenderCacheHandler::getRenderContainerAtRelFrame(
                                const int &frame) {
    if(mNoCache) {
        if(mCurrentRenderContainer->getFrame() != frame) return NULL;
        return mCurrentRenderContainer;
    }
    return getRenderCacheRangeContainingRelFrame(frame)->
            getRenderContainerAtRelFrame(frame);
}

CacheBoundingBoxRenderContainer *
RenderCacheHandler::createNewRenderContainerAtRelFrame(
                                const int &frame) {
    return getRenderCacheRangeContainingRelFrame(frame)->
            createNewRenderContainerAtRelFrame(frame);
}

void RenderCacheHandler::setNoCache(const bool &noCache) {
    mNoCache = noCache;
    clearAllCache();
    if(noCache) {
        setCurrentRenderContainerVar(new CacheBoundingBoxRenderContainer());
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
                                        (AnimationRect*)mDurationRect);
    }
}

void RenderCacheHandler::clearRenderRangeforAnimationRange() {
    if(mAnimationRenderCacheRange != NULL) {
        applyChanges();

        delete mAnimationRenderCacheRange;

        mAnimationRenderCacheRange = NULL;
    }
}

void RenderCacheHandler::clearAllCache() {
    foreach(RenderCacheRange *range, mRenderCacheRange) {
        range->clearCache();
    }
    if(mAnimationRenderCacheRange != NULL) {
        mAnimationRenderCacheRange->clearCache();
    }
    //setCurrentRenderContainerVar(NULL);
}

void RenderCacheHandler::addRangeNeedingUpdate(const int &min,
                                               const int &max) {
    RenderCacheRange *first = getRenderCacheRangeContainingRelFrame(min,
                                                                    false);
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
                                                     key));
}

void RenderCacheHandler::addChangedKey(Key *key) {
    addRenderCacheRangeChange(RenderCacheRangeChange(CHANGE_KEY,
                                                     key->getRelFrame()));
}

void RenderCacheHandler::updateCurrentRenderContainerFromFrame(
                                const int &relFrame) {
    if(mNoCache) return;
    CacheBoundingBoxRenderContainer *contAtFrame =
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
    if(mNoCache) return false;
    CacheBoundingBoxRenderContainer *cont =
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
                            CacheBoundingBoxRenderContainer *cont) {
    if(mCurrentRenderContainer != NULL) {
        mCurrentRenderContainer->decNumberPointers();
    }
    mCurrentRenderContainer = cont;
    if(mCurrentRenderContainer != NULL) {
        mCurrentRenderContainer->incNumberPointers();
        mCurrentRenderContainer->thisAccessed();
    }
}

void RenderCacheHandler::divideRenderCacheRangeAtRelFrame(
                                const int &divideRelFrame,
                                Key *newKey) {
    RenderCacheRange *oldRange =
            getRenderCacheRangeContainingRelFrame(divideRelFrame);
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
                                                    relFrame);
    mRenderCacheRange.insert(oldId, range1);
    RenderCacheRange *range2 = new RenderCacheRange(newKey,
                                                    maxKey,
                                                    relFrame,
                                                    oldRange->getMaxRelFrame());
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
    qDebug() << range->getMinRelFrame() << range->getMaxRelFrame();
    mRangesNeedingUpdate << range;
}

void RenderCacheHandler::drawCacheOnTimeline(QPainter *p,
                           const qreal &pixelsPerFrame,
                           const qreal &drawY,
                           const int &startFrame,
                           const int &endFrame) {
    p->setBrush(QColor(0, 255, 0, 75));
    p->setPen(Qt::NoPen);
    int lastDrawnFrame = 0;
    int lastDrawX = 0;
    RenderCacheRange *first = getRenderCacheRangeContainingRelFrame(startFrame,
                                                                    false);
    first->drawCacheOnTimeline(p,
                               pixelsPerFrame,
                               drawY,
                               startFrame,
                               endFrame,
                               &lastDrawnFrame,
                               &lastDrawX);
    int currId = mRenderCacheRange.indexOf(first) + 1;
    while(currId < mRenderCacheRange.count()) {
        RenderCacheRange *nextRange = mRenderCacheRange.at(currId);
        if(nextRange->getMinRelFrame() > endFrame) return;
        nextRange->drawCacheOnTimeline(p,
                                       pixelsPerFrame,
                                       drawY,
                                       startFrame,
                                       endFrame,
                                       &lastDrawnFrame,
                                       &lastDrawX);
        currId++;
    }
    if(mAnimationRenderCacheRange == NULL) return;
    lastDrawnFrame = 0;
    lastDrawX = 0;
    mAnimationRenderCacheRange->drawCacheOnTimeline(
                                   p,
                                   pixelsPerFrame,
                                   drawY,
                                   startFrame,
                                   endFrame,
                                   &lastDrawnFrame,
                                   &lastDrawX);
}

AnimationRenderCacheRange::AnimationRenderCacheRange(
        AnimationRect *durationRect) :
    RenderCacheRange(NULL, NULL,
                     durationRect->getMinAnimationFrameAsRelFrame(),
                     durationRect->getMaxAnimationFrameAsRelFrame()) {
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
