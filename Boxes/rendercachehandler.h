#ifndef RENDERCACHEHANDLER_H
#define RENDERCACHEHANDLER_H
class Key;
#include <QtCore>
class QPainter;
class BoundingBoxRenderContainer;
class CacheBoundingBoxRenderContainer;
class DurationRectangle;

enum RenderCacheRangeChangeType {
//    MOVE_KEY,
    REMOVE_KEY,
    ADD_KEY,
    CHANGE_KEY
};

struct RenderCacheRangeChange {
    RenderCacheRangeChange(const RenderCacheRangeChangeType &typeT,
                           const int &relFrameT);

    RenderCacheRangeChange(const RenderCacheRangeChangeType &typeT,
                           Key *keyToAdd);

    RenderCacheRangeChangeType type;
    int relFrame;
    Key *key;
};

class RenderCacheHandler;

class RenderCacheRange {
public:
    RenderCacheRange(Key *minKey,
                     Key *maxKey,
                     const int &minFrame,
                     const int &maxFrame,
                     RenderCacheHandler *parentHandler);

    virtual ~RenderCacheRange();

    void clearCacheAboveRelFrame(const int &min);
    void clearCacheBelowRelFrame(const int &max);

    void setInternalDifferencesPresent(const bool &bT);

    bool areInternalDifferencesPresent() const;

    bool relFrameInRange(const int &relFrame);

    void clearCache();

    const int &getMaxRelFrame();

    const int &getMinRelFrame();

    virtual void updateAfterKeysChanged();

    void setMinKeyTMP(Key *minKey);

    void setMaxKeyTMP(Key *maxKey);

    void setMaxRelFrame(const int &maxRelFrame);

    void setMinRelFrame(const int &minRelFrame);

    Key *getMinKey();

    Key *getMaxKey();

    virtual bool isBlocked() { return false; }
protected:
    bool mInternalDifferences = false;

    int mMinRelFrame;
    int mMaxRelFrame;

    Key *mMinKey = NULL;
    Key *mMaxKey = NULL;
    RenderCacheHandler *mParentHandler = NULL;
};

class AnimationRect;

class AnimationRenderCacheRange : public RenderCacheRange {
public:
    AnimationRenderCacheRange(AnimationRect *durationRect,
                              RenderCacheHandler *parentHandler);

    bool isBlocked() { return true; }

    void updateAfterKeysChanged() {}

    void addInternalKey(Key *key);
    void removeInternalKey(Key *key);

    void updateMaxKey() {
        mMaxKey = getLastInternalKey();
    }

    void updateMinKey() {
        mMinKey = getFirstInternalKey();
    }

    void addAllInternalKeysToHandler(RenderCacheHandler *handler);
private:
    Key *getLastInternalKey() {
        if(mInternalKeys.isEmpty()) return NULL;
        return mInternalKeys.last();
    }

    Key *getFirstInternalKey() {
        if(mInternalKeys.isEmpty()) return NULL;
        return mInternalKeys.first();
    }
    int getKeyInsertIdForFrame(const int &frame);

    QList<Key*> mInternalKeys;
    AnimationRect *mDurationRect = NULL;
};

class BoundingBox;
class RenderCacheHandler : public QObject {
    Q_OBJECT
public:
    RenderCacheHandler();

    void applyChanges();

    void addRenderCacheRangeChange(const RenderCacheRangeChange &change);

    CacheBoundingBoxRenderContainer *getRenderContainerAtRelFrame(
                                    const int &frame);

    CacheBoundingBoxRenderContainer *createNewRenderContainerAtRelFrame(
                                    const int &frame);

    void clearCache();

    void addRangeNeedingUpdate(const int &min, const int &max);

    void addAddedKey(Key *key);

    void addRemovedKey(Key *key);

    void addChangedKey(Key *key);

    void updateCurrentRenderContainerFromFrame(const int &relFrame);

    bool updateCurrentRenderContainerFromFrameIfNotNull(const int &relFrame);

    void duplicateCurrentRenderContainerFrom(BoundingBoxRenderContainer *cont);

    void updateCurrentRenderContainerTransform(const QMatrix &trans);

    void drawCurrentRenderContainer(QPainter *p);

    void drawCacheOnTimeline(QPainter *p,
                             const qreal &pixelsPerFrame,
                             const qreal &drawY,
                             const int &startFrame,
                             const int &endFrame);
    void setNoCache(const bool &noCache);

    bool isRelFrameInDurationRectAnimationRange(const int &frame);
    void setupRenderRangeforAnimationRange();

    void divideRenderCacheRangeAtRelFrame(const int &divideRelFrame,
                                          Key *newKey);
    bool isThereBarrierAtRelFrame(const int &frame);
    void setDurationRectangle(DurationRectangle *durRect);

    void removeRenderContainer(CacheBoundingBoxRenderContainer *cont);
    void clearCacheForRelFrameRange(const int &minFrame,
                                    const int &maxFrame);
    bool areInternalDifferencesPresentFromAll(const int &relFrame);

    void addInfluencingHandler(RenderCacheHandler *handler);
    void removeInfluencingHandler(RenderCacheHandler *handler);
    void setParentBox(BoundingBox *parentBox) {
        mParentBox = parentBox;
    }

    void relRangeToAbsRange(int *minFrame,
                            int *maxFrame) {
        *minFrame = relFrameToAbsFrame(*minFrame);
        *maxFrame = relFrameToAbsFrame(*maxFrame);
    }

    void absRangeToRelRange(int *minFrame,
                            int *maxFrame) {
        *minFrame = absFrameToRelFrame(*minFrame);
        *maxFrame = absFrameToRelFrame(*maxFrame);
    }

    int relFrameToAbsFrame(const int &relFrame);
    int absFrameToRelFrame(const int &absFrame);
signals:
    void clearedCacheForAbsFrameRange(int, int);
public slots:
    void clearCacheForAbsFrameRange(const int &minFrame,
                                    const int &maxFrame);
    void updateVisibilityRange();
    void updateAnimationRenderCacheRange();
private:
    BoundingBox *mParentBox = NULL;
    QList<RenderCacheHandler*> mInfluencingHandlers;
    QList<CacheBoundingBoxRenderContainer*> mRenderContainers;

    int mMinRelFrame = INT_MIN;
    int mMaxRelFrame = INT_MAX;

    void clearRenderRangeforAnimationRange();

    AnimationRenderCacheRange *mAnimationRenderCacheRange = NULL;
    bool mNoCache = false;
    DurationRectangle *mDurationRect = NULL;

    void setCurrentRenderContainerVar(CacheBoundingBoxRenderContainer *cont);

    CacheBoundingBoxRenderContainer *mCurrentRenderContainer = NULL;

    void divideRenderCacheRange(RenderCacheRange *oldRange,
                                const int &relFrame,
                                Key *newKey);

    void mergeRenderCacheRanges(RenderCacheRange *prevRange,
                              RenderCacheRange *nextRange);

    void getRenderCacheRangesWithBarrierAtRelFrame(
                            RenderCacheRange **prevRange,
                            RenderCacheRange **nextRange,
                            const int &relFrame);

    void removeRangesBarrierAtRelFrame(const int &relFrame);

    void updateAfterBarrierValueChanged(const int &barrierRelFrame);

    RenderCacheRange *getRenderCacheRangeContainingRelFrame(
                        const int &relFrame,
                        const bool &considerAnimationRange = true);

    void removeRenderCacheRangeFromList(RenderCacheRange *range);

    void applyChange(const RenderCacheRangeChange &change);

    void removeRangeNeedingUpdate(RenderCacheRange *range);

    void addRangeNeedingUpdate(RenderCacheRange *range);

    int getMaxRelFrameForContainerAtRel(const int &relFrame);
    int getMinRelFrameForContainerAtRel(const int &relFrame);

    QList<RenderCacheRangeChange> mRenderCacheRangeRemovals;
    QList<RenderCacheRangeChange> mRenderCacheRangeAdds;
    QList<RenderCacheRangeChange> mRenderCacheRangeValueChanges;

    QList<RenderCacheRange*> mRangesNeedingUpdate;

    QList<RenderCacheRange*> mRenderCacheRange;
    int getRenderContainterInsertIdAtRelFrame(const int &relFrame);
    bool getRenderContainterIdAtRelFrame(const int &relFrame, int *id);
    void insertRenderContainer(CacheBoundingBoxRenderContainer *cont);
};

#endif // RENDERCACHEHANDLER_H
