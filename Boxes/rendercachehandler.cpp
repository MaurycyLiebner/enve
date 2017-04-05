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
        BoundingBoxRenderContainer *cont = mRenderContainers.at(guess);
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
        BoundingBoxRenderContainer *cont = mRenderContainers.at(guess);
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

void RenderCacheRange::insertRenderContainer(BoundingBoxRenderContainer *cont) {
    int contId = getRenderContainterInsertIdAtRelFrame(cont->getFrame());
    mRenderContainers.insert(contId, cont);
    cont->incNumberPointers();
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
#include <QDebug>
BoundingBoxRenderContainer *
RenderCacheRange::createNewRenderContainerAtRelFrame(const int &frame) {
    BoundingBoxRenderContainer *cont = new BoundingBoxRenderContainer();
    cont->setFrame(frame);
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
                                           const int &endFrame,
                                           int *lastDrawnRelToStartFrameP,
                                           int *lastDrawFrameRightPosP) {
    int lastDrawnRelToStartFrame = *lastDrawnRelToStartFrameP;
    int lastDrawFrameRightPos = *lastDrawFrameRightPosP;
    if(mInternalDifferences) {
        foreach(BoundingBoxRenderContainer *cont, mRenderContainers) {
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

BoundingBoxRenderContainer *RenderCacheHandler::getRenderContainerAtRelFrame(
                                const int &frame) {
    if(mNoCache) {
        if(mCurrentRenderContainer->getFrame() != frame) return NULL;
        return mCurrentRenderContainer;
    }
    return getRenderCacheRangeContainingRelFrame(frame)->
            getRenderContainerAtRelFrame(frame);
}

BoundingBoxRenderContainer *RenderCacheHandler::createNewRenderContainerAtRelFrame(
                                const int &frame) {
    return getRenderCacheRangeContainingRelFrame(frame)->
            createNewRenderContainerAtRelFrame(frame);
}

void RenderCacheHandler::setNoCache(const bool &noCache) {
    mNoCache = noCache;
    clearAllCache();
    if(noCache) {
        setCurrentRenderContainerVar(new BoundingBoxRenderContainer());
    }
}

void RenderCacheHandler::setupRenderRangeforAnimationRange() {
    if(mDurationRect == NULL) return;
    if(!mDurationRect->hasAnimationFrameRange()) return;
    if(mAnimationRangeSetup) {
        clearRenderRangeforAnimationRange();
    }
    mAnimationRangeSetup = true;
    int durRectMin = mDurationRect->getMinAnimationFrameAsRelFrame();
    int durRectMax = mDurationRect->getMaxAnimationFrameAsRelFrame();
    if(!isThereBarrierAtRelFrame(durRectMin)) {
        divideRenderCacheRangeAtRelFrame(durRectMin, NULL);
    }
    if(!isThereBarrierAtRelFrame(durRectMax)) {
        divideRenderCacheRangeAtRelFrame(durRectMax, NULL);
    }

    QList<Key*> newRangeInternalKeys;
    int idAtMin = mRenderCacheRange.indexOf(
                getRenderCacheRangeContainingRelFrame(durRectMin));
    int idAtMax = mRenderCacheRange.indexOf(
                getRenderCacheRangeContainingRelFrame(durRectMax));
    for(int i = idAtMin; i < idAtMax; i++) {
        RenderCacheRange *iRange = mRenderCacheRange.takeAt(idAtMin);
        Key *maxKey = iRange->getMaxKey();
        Key *minKey = iRange->getMinKey();
        if(maxKey != NULL) {
            newRangeInternalKeys << maxKey;
        }
        if(minKey != NULL) {
            newRangeInternalKeys << minKey;
        }
        delete iRange;
    }

    AnimationRenderCacheRange *newRange = new AnimationRenderCacheRange(
                                                            mDurationRect);
    foreach(Key *key, newRangeInternalKeys) {
        newRange->addInternalKey(key);
    }
    mRenderCacheRange.insert(idAtMin, newRange);
}

void RenderCacheHandler::clearRenderRangeforAnimationRange() {
    if(mAnimationRangeSetup) {
        mAnimationRangeSetup = false;
        foreach(RenderCacheRange *range, mRenderCacheRange) {
            if(range->isBlocked()) {
                AnimationRenderCacheRange *aRange =
                        (AnimationRenderCacheRange*)range;
                int oldId = mRenderCacheRange.indexOf(range);
                RenderCacheRange *prevRange = mRenderCacheRange.at(oldId - 1);
                RenderCacheRange *nextRange = mRenderCacheRange.at(oldId + 1);
                mRenderCacheRange.removeOne(range);
                mRenderCacheRange.removeOne(prevRange);
                mRenderCacheRange.removeOne(nextRange);

                RenderCacheRange *newRange = new RenderCacheRange(
                                                prevRange->getMinKey(),
                                                nextRange->getMaxKey(),
                                                prevRange->getMinRelFrame(),
                                                nextRange->getMaxRelFrame());
                mRenderCacheRange.insert(oldId - 1, newRange);

                aRange->addAllInternalKeysToHandler(this);

                delete aRange;
                delete prevRange;
                delete nextRange;

                return;
            }
        }
    }
}

void RenderCacheHandler::clearAllCache() {
    foreach(RenderCacheRange *range, mRenderCacheRange) {
        range->clearCache();
    }
    setCurrentRenderContainerVar(NULL);
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
                                                     key));
}

void RenderCacheHandler::addChangedKey(Key *key) {
    addRenderCacheRangeChange(RenderCacheRangeChange(CHANGE_KEY,
                                                     key->getRelFrame()));
}

void RenderCacheHandler::updateCurrentRenderContainerFromFrame(
                                const int &relFrame) {
    if(mNoCache) return;
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
    if(mNoCache) return false;
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
    divideRenderCacheRange(oldRange, divideRelFrame, newKey);
}

void RenderCacheHandler::divideRenderCacheRange(RenderCacheRange *oldRange,
                                                const int &relFrame,
                                                Key *newKey) {
    if(oldRange->isBlocked()) {
        int oldId = mRenderCacheRange.indexOf(oldRange);
        ((AnimationRenderCacheRange*)oldRange)->addInternalKey(newKey);
        RenderCacheRange *prevRange = mRenderCacheRange.at(oldId - 1);
        prevRange->setMaxKeyTMP(oldRange->getMinKey());
        RenderCacheRange *nextRange = mRenderCacheRange.at(oldId + 1);
        if(nextRange->getMinKey() != NULL) {
            if(nextRange->getMinKey()->getRelFrame() !=
               nextRange->getMinRelFrame()) {
                nextRange->setMinKeyTMP(oldRange->getMaxKey());
            }
        } else {
            nextRange->setMinKeyTMP(oldRange->getMaxKey());
        }

        addRangeNeedingUpdate(oldRange);
        addRangeNeedingUpdate(nextRange);
        addRangeNeedingUpdate(prevRange);
    } else {
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

void RenderCacheHandler::getRenderCacheRangesWithBarrierAtRelFrame(
        RenderCacheRange **prevRange,
        RenderCacheRange **nextRange,
        const int &relFrame) {
    *prevRange = getRenderCacheRangeContainingRelFrame(relFrame - 1);
    *nextRange = getRenderCacheRangeContainingRelFrame(relFrame);
}

void RenderCacheHandler::removeRangesBarrierAtRelFrame(const int &relFrame,
                                                       Key *key) {
    RenderCacheRange *prevRange;
    RenderCacheRange *nextRange;
    getRenderCacheRangesWithBarrierAtRelFrame(&prevRange,
                                              &nextRange,
                                              relFrame);
    if(prevRange->isBlocked() || nextRange->isBlocked()) {
        AnimationRenderCacheRange *animationRange;
        if(prevRange->isBlocked()) {
            animationRange = (AnimationRenderCacheRange*)prevRange;
        } else {
            animationRange = (AnimationRenderCacheRange*)nextRange;
        }
        animationRange->removeInternalKey(key);
        int oldId = mRenderCacheRange.indexOf(animationRange);
        RenderCacheRange *prevRange2 = mRenderCacheRange.at(oldId - 1);
        prevRange2->setMaxKeyTMP(animationRange->getMinKey());
        RenderCacheRange *nextRange2 = mRenderCacheRange.at(oldId + 1);
        if(nextRange2->getMinKey() != NULL) {
            if(nextRange2->getMinKey()->getRelFrame() !=
               nextRange2->getMinRelFrame()) {
                nextRange2->setMinKeyTMP(animationRange->getMaxKey());
            }
        } else {
            nextRange2->setMinKeyTMP(animationRange->getMaxKey());
        }

        addRangeNeedingUpdate(animationRange);
        addRangeNeedingUpdate(nextRange2);
        addRangeNeedingUpdate(prevRange2);
    } else {
        mergeRenderCacheRanges(prevRange, nextRange);
    }
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
        removeRangesBarrierAtRelFrame(change.relFrame, change.key);
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
    p->setBrush(QColor(0, 255, 0, 75));
    p->setPen(Qt::NoPen);
    int lastDrawnFrame = 0;
    int lastDrawX = 0;
    RenderCacheRange *first = getRenderCacheRangeContainingRelFrame(startFrame);
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
}

AnimationRenderCacheRange::AnimationRenderCacheRange(
        DurationRectangle *durationRect) :
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
