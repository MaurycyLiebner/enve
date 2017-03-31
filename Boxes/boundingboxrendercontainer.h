#ifndef BOUNDINGBOXRENDERCONTAINER_H
#define BOUNDINGBOXRENDERCONTAINER_H
class BoundingBox;
#include <QImage>
#include "smartpointertarget.h"

class BoundingBoxRenderContainer : public SmartPointerTarget {
public:
    BoundingBoxRenderContainer();

    void draw(QPainter *p);

    void updatePaintTransformGivenNewCombinedTransform(
            const QMatrix &combinedTransform);

    void replaceImage(const QImage &img);

    void setTransform(const QMatrix &transform);

    void setBoundingRect(const QRectF &rect);

    const QMatrix &getTransform() const;

    const QImage &getImage() const;

    const QMatrix &getPaintTransform() const;

    const QRectF &getBoundingRect() const;

    const qreal &getResolutionPercent() const;

    const int &getFrame() const;

    void setFrame(const int &frame);

    void updateVariables(const QMatrix &combinedTransform,
                         const qreal &effectsMargin,
                         const qreal &resolutionPer,
                         BoundingBox *target);

    void duplicateFrom(BoundingBoxRenderContainer *src);

    void setVariables(const QMatrix &transform,
                      const QMatrix &paintTransform,
                      const QRectF &rect,
                      const QImage &img,
                      const int &frame, const qreal &res);
    void drawWithoutTransform(QPainter *p);
private:
    int mFrame = 0;
    qreal mResolutionPercent;
    QMatrix mTransform;
    QMatrix mPaintTransform;
    QRectF mBoundingRect;
    QImage mImage;
};

enum InfluenceRangeChangeType {
//    MOVE_KEY,
    REMOVE_KEY,
    ADD_KEY,
    CHANGE_VALUE
};

#include "key.h"

struct InfluenceRangeChange {
    InfluenceRangeChange(const InfluenceRangeChangeType &typeT,
                         const int &relFrameT) {
        type = typeT;
        relFrame = relFrameT;
        key = NULL;
    }

    InfluenceRangeChange(const InfluenceRangeChangeType &typeT,
                         Key *keyToAdd) {
        type = typeT;
        relFrame = keyToAdd->getRelFrame();
        key = keyToAdd;
    }

    InfluenceRangeChangeType type;
    int relFrame;
    Key *key;
};

class InfluenceRange {
public:
    InfluenceRange(Key *minKey,
                   Key *maxKey) {
        mMinKey = minKey;
        mMaxKey = maxKey;

        updateMinRelFrameFromKey();
        updateMaxRelFrameFromKey();

        updateAfterKeysChanged();
    }

    InfluenceRange(Key *minKey,
                   Key *maxKey,
                   const int &minFrame,
                   const int &maxFrame) {
        mMinKey = minKey;
        mMaxKey = maxKey;

        mMinRelFrame = minFrame;
        mMaxRelFrame = maxFrame;
    }

    ~InfluenceRange() {
        clearCache();
    }

    void updateMinRelFrameFromKey() {
        if(mMinKey == NULL) {
            mMinRelFrame = INT_MIN;
        } else {
            mMinRelFrame = mMinKey->getRelFrame();
        }
    }

    void updateMaxRelFrameFromKey() {
        if(mMaxKey == NULL) {
            mMaxRelFrame = INT_MIN;
        } else {
            mMaxRelFrame = mMaxKey->getRelFrame();
        }
    }

    void setInternalDifferencesPresent(const bool &bT) {
        if(bT == mInternalDifferences) return;
        mInternalDifferences = bT;
        clearCache();
    }

    bool areInternalDifferencesPresent() const {
        return mInternalDifferences;
    }

    bool relFrameInRange(const int &relFrame) {
        return relFrame >= getMinRelFrame() && relFrame < getMaxRelFrame();
    }

    bool getRenderContainterIdAtRelFrame(const int &relFrame, int *id) {
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

    int getRenderContainterInsertIdAtRelFrame(const int &relFrame) {
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

    BoundingBoxRenderContainer *getRenderContainerAtRelFrame(
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

    BoundingBoxRenderContainer *createNewRenderContainerAtRelFrame(
                                const int &frame) {
        BoundingBoxRenderContainer *cont = new BoundingBoxRenderContainer();
        cont->incNumberPointers();
        if(mInternalDifferences) {
            int contId = getRenderContainterInsertIdAtRelFrame(frame);
            mRenderContainers.insert(contId, cont);
        } else {
            mRenderContainers.append(cont);
        }
        return cont;
    }

    void clearCache() {
        foreach(BoundingBoxRenderContainer *cont, mRenderContainers) {
            cont->decNumberPointers();
        }

        mRenderContainers.clear();
    }

    const int &getMaxRelFrame() {
        return mMaxRelFrame;
    }

    const int &getMinRelFrame() {
        return mMinRelFrame;
    }

    void updateAfterKeysChanged() {
        bool keysDiffer;
        if(mMinKey == NULL || mMaxKey == NULL) {
            keysDiffer = false;
        } else {
            keysDiffer = mMinKey->differsFromKey(mMaxKey);
        }
        setInternalDifferencesPresent(keysDiffer);
    }

    void setMinKeyTMP(Key *minKey) {
        mMinKey = minKey;
    }

    void setMaxKeyTMP(Key *maxKey) {
        mMaxKey = maxKey;
    }

    void setMaxRelFrame(const int &maxRelFrame) {
        mMaxRelFrame = maxRelFrame;
    }

    void setMinRelFrame(const int &minRelFrame) {
        mMinRelFrame = minRelFrame;
    }

    Key *getMinKey() {
        return mMinKey;
    }

    Key *getMaxKey() {
        return mMaxKey;
    }
private:
    QList<BoundingBoxRenderContainer*> mRenderContainers;
    bool mInternalDifferences;

    int mMinRelFrame;
    int mMaxRelFrame;

    Key *mMinKey;
    Key *mMaxKey;
};

#include "Animators/complexanimator.h"
class InfluenceRangeHandler {
public:
    InfluenceRangeHandler() {
        mInfluenceRange << new InfluenceRange(NULL, NULL);
    }

    InfluenceRange *getInfluenceRangeContainingRelFrame(
                                        const int &relFrame) {
        int minId = 0;
        int maxId = mInfluenceRange.count() - 1;

        while(minId < maxId) {
            int guess = (minId + maxId)/2;
            if(guess == maxId) { guess = minId; }
            else if(guess == minId) { guess = maxId; }
            InfluenceRange *guessRange = mInfluenceRange.at(guess);
            if(guessRange->relFrameInRange(relFrame)) return guessRange;

            if(guessRange->getMinRelFrame() > relFrame) {
                maxId = guess;
            } else {
                minId = guess;
            }
        }
    }

    void updateCurrentRenderContainerFromRelFrame() {
        if(mCurrentRenderContainer != NULL) {
            mCurrentRenderContainer->decNumberPointers();
        }
        mCurrentRenderContainer =
                mCurrentInfluenceRange->
                    getRenderContainerAtRelFrame(mCurrentRelFrame);
        mCurrentRenderContainer->incNumberPointers();
    }

    void updateCurrentInfluenceRangeFromRelFrame() {
        if(mCurrentInfluenceRange != NULL) {
            if(mCurrentInfluenceRange->relFrameInRange(mCurrentRelFrame)) {
                return;
            }
        }
        mCurrentInfluenceRange =
                getInfluenceRangeContainingRelFrame(mCurrentRelFrame);
    }

    void setCurrentRelFrame(const int &relFrame) {
        mCurrentRelFrame = relFrame;
        updateCurrentInfluenceRangeFromRelFrame();
        updateCurrentRenderContainerFromRelFrame();
    }

    void divideInfluenceRangeAtRelFrame(const int &divideRelFrame,
                                        Key *newKey) {
        InfluenceRange *oldRange =
                getInfluenceRangeContainingRelFrame(divideRelFrame);
        divideInfluenceRange(oldRange, newKey);
    }

    void divideInfluenceRange(InfluenceRange *oldRange,
                              Key *newKey) {
        if(oldRange == mCurrentInfluenceRange) {
            mCurrentInfluenceRange = NULL;
        }
        int oldId = mInfluenceRange.indexOf(oldRange);
        Key *minKey = oldRange->getMinKey();
        Key *maxKey = oldRange->getMaxKey();
        removeRangeNeedingUpdate(oldRange);
        InfluenceRange *range1 = new InfluenceRange(minKey,
                                                    newKey,
                                                    oldRange->getMinRelFrame(),
                                                    newKey->getRelFrame());
        mInfluenceRange.insert(oldId, range1);
        InfluenceRange *range2 = new InfluenceRange(newKey,
                                                    maxKey,
                                                    newKey->getRelFrame(),
                                                    oldRange->getMaxRelFrame());
        mInfluenceRange.insert(oldId + 1, range2);
        delete oldRange;

        addRangeNeedingUpdate(range1);
        addRangeNeedingUpdate(range2);
    }

    void mergeInfluenceRanges(InfluenceRange *prevRange,
                              InfluenceRange *nextRange) {
        prevRange->setMaxRelFrame(nextRange->getMaxRelFrame());
        prevRange->setMaxKeyTMP(nextRange->getMinKey());
        removeInfluenceRangeFromList(nextRange);
        removeRangeNeedingUpdate(nextRange);
        delete nextRange;
    }

//    void moveInfluenceRangesBarrier(InfluenceRange *prevRange,
//                                    InfluenceRange *nextRange,
//                                    const int &moveBy) {
//        prevRange->setMaxRelFrame(prevRange->getMaxRelFrame() + moveBy);
//        nextRange->setMinRelFrame(nextRange->getMinRelFrame() + moveBy);
//    }

    void getInfluenceRangesWithBarrierAtRelFrame(
                            InfluenceRange **prevRange,
                            InfluenceRange **nextRange,
                            const int &relFrame) {
        *prevRange = getInfluenceRangeContainingRelFrame(relFrame - 1);
        *nextRange = getInfluenceRangeContainingRelFrame(relFrame);
    }

    void removeRangesBarrierAtRelFrame(const int &relFrame) {
        InfluenceRange *prevRange;
        InfluenceRange *nextRange;
        getInfluenceRangesWithBarrierAtRelFrame(&prevRange,
                                                &nextRange,
                                                relFrame);
        mergeInfluenceRanges(prevRange, nextRange);
    }

    BoundingBoxRenderContainer *getCurrentRenderContainer() {
        return mCurrentRenderContainer;
    }

    int getRenderContainterInsertIdForInfluenceRange(InfluenceRange *range) {
        int minId = 0;
        int maxId = mInfluenceRange.count() - 1;

        while(minId < maxId) {
            int guess = (minId + maxId)/2;
            if(guess == maxId) {
                guess = minId;
            } else if(guess == minId) {
                guess = maxId;
            }
            InfluenceRange *cont = mInfluenceRange.at(guess);
            if(cont->getMinRelFrame() > range->getMaxRelFrame()) {
                maxId = guess;
            } else if(cont->getMaxRelFrame() < range->getMinRelFrame()) {
                minId = guess;
            } else {
                return guess;
            }
        }
        return 0;
    }

    void addInfluenceRange(InfluenceRange *range) {
        mInfluenceRange.insert(
                    getRenderContainterInsertIdForInfluenceRange(range),
                    range);
    }

    void removeInfluenceRangeFromList(InfluenceRange *range) {
        if(mCurrentInfluenceRange == range) mCurrentInfluenceRange = NULL;
        mInfluenceRange.removeOne(range);
    }

    void updateAfterBarrierValueChanged(const int &barrierRelFrame) {
        InfluenceRange *prevRange;
        InfluenceRange *nextRange;
        getInfluenceRangesWithBarrierAtRelFrame(&prevRange,
                                                &nextRange,
                                                barrierRelFrame);
        addRangeNeedingUpdate(prevRange);
        addRangeNeedingUpdate(nextRange);
    }

    void applyChange(const InfluenceRangeChange &change) {
        if(change.type == REMOVE_KEY) {
            removeRangesBarrierAtRelFrame(change.relFrame);
        } else if(change.type == ADD_KEY) {
            divideInfluenceRangeAtRelFrame(change.relFrame, change.key);
        } else if(change.type == CHANGE_VALUE) {
            updateAfterBarrierValueChanged(change.relFrame);
        }
    }

    void applyChanges() {
        foreach(const InfluenceRangeChange &change, mInfluenceRangeRemovals) {
            applyChange(change);
        }
        mInfluenceRangeRemovals.clear();
        foreach(const InfluenceRangeChange &change, mInfluenceRangeAdds) {
            applyChange(change);
        }
        mInfluenceRangeAdds.clear();
        foreach(const InfluenceRangeChange &change, mInfluenceRangeValueChanges) {
            applyChange(change);
        }
        mInfluenceRangeValueChanges.clear();

        foreach(InfluenceRange *rangeNeedingUpdate, mRangesNeedingUpdate) {
            rangeNeedingUpdate->updateAfterKeysChanged();
        }
        mRangesNeedingUpdate.clear();

        updateCurrentInfluenceRangeFromRelFrame();
        updateCurrentRenderContainerFromRelFrame();
    }

    void removeRangeNeedingUpdate(InfluenceRange *range) {
        mRangesNeedingUpdate.removeOne(range);
    }

    void addRangeNeedingUpdate(InfluenceRange *range) {
        if(mRangesNeedingUpdate.contains(range)) return;
        mRangesNeedingUpdate << range;
    }

    void addInfluenceRangeChange(const InfluenceRangeChange &change) {
        if(change.type == ADD_KEY) {
            mInfluenceRangeAdds << change;
        } else if(change.type == REMOVE_KEY) {
            mInfluenceRangeRemovals << change;
        } else if(change.type == CHANGE_VALUE) {
            mInfluenceRangeValueChanges << change;
        }
    }

private:
    QList<InfluenceRangeChange> mInfluenceRangeRemovals;
    QList<InfluenceRangeChange> mInfluenceRangeAdds;
    QList<InfluenceRangeChange> mInfluenceRangeValueChanges;
    QList<InfluenceRange*> mRangesNeedingUpdate;

    int mCurrentRelFrame = 0;
    InfluenceRange *mCurrentInfluenceRange = NULL;
    BoundingBoxRenderContainer *mCurrentRenderContainer = NULL;
    QList<InfluenceRange*> mInfluenceRange;
};

#endif // BOUNDINGBOXRENDERCONTAINER_H
