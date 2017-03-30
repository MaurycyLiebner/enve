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

class InfluenceRange {
public:
    InfluenceRange(const bool &iD,
                   const int &minRF,
                   const int &maxRF) {
        mInternalDifferences = iD;
        mMinRelFrame = minRF;
        mMaxRelFrame = maxRF;
    }
    ~InfluenceRange() {
        clearCache();
    }

    void setInternalDifferencesPresent(const bool &bT) {
        mInternalDifferences = bT;
        clearCache();
    }

    bool relFrameInRange(const int &relFrame) {
        return relFrame >= mMinRelFrame && relFrame <= mMaxRelFrame;
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

    int getMaxRelFrame() {
        return mMaxRelFrame;
    }

    int getMinRelFrame() {
        return mMinRelFrame;
    }

private:
    QList<BoundingBoxRenderContainer*> mRenderContainers;
    bool mInternalDifferences;
    int mMinRelFrame;
    int mMaxRelFrame;
};

class InfluenceRangeHandler {
public:
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

//    void divideInfluenceRangeAtRelFrame(const int &divideRelFrame) {
//        InfluenceRange *oldRange =
//                getInfluenceRangeContainingRelFrame(divideRelFrame);
//        divideInfluenceRange(oldRange);
//    }

//    void divideInfluenceRange(InfluenceRange *oldRange) {
//        if(oldRange == mCurrentInfluenceRange) {
//            mCurrentInfluenceRange = NULL;
//        }
//        int oldId = mInfluenceRange.indexOf(oldRange);
//        int minFrame = oldRange->getMinRelFrame();
//        int maxFrame = oldRange->getMaxRelFrame();
//        delete oldRange;
//        mInfluenceRange.insert(oldId, new InfluenceRange(true,
//                                                         minFrame,
//                                                         divideRelFrame));
//        mInfluenceRange.insert(oldId + 1, new InfluenceRange(true,
//                                                             divideRelFrame,
//                                                             maxFrame));

//        updateCurrentInfluenceRangeFromRelFrame();
//        updateCurrentRenderContainerFromRelFrame();
//    }

    BoundingBoxRenderContainer *getCurrentRenderContainer() {
        return mCurrentRenderContainer;
    }

    void addChangedRelFrame(const int &changedRelFrame) {
        mChangedRelFrames << changedRelFrame;
    }

    void applyAllChanges() {

        mChangedRelFrames.clear();
    }

    int getRenderContainterInsertIdForInfluenceRange(InfluenceRange *range) {
        int minId = 0;
        int maxId = mInfluenceRange.count() - 1;

        while(minId < maxId) {
            int guess = (minId + maxId)/2;
            if(guess == maxId || guess == minId) return maxId;
            InfluenceRange *cont = mInfluenceRange.at(guess);
            int contFrame = cont->getMinRelFrame();
            if(contFrame > range->getMaxRelFrame()) {
                maxId = guess;
            } else if(contFrame < range->getMinRelFrame()) {
                minId = guess;
            } else {
                return guess;
            }
        }
        return false;
    }

    void addInfluenceRange(InfluenceRange *range) {
        mInfluenceRange.insert(
                    getRenderContainterInsertIdForInfluenceRange(range),
                    range);
    }

    void removeInfluenceRange(InfluenceRange *range) {
        mInfluenceRange.removeOne(range);
    }

private:
    int mCurrentRelFrame = 0;
    InfluenceRange *mCurrentInfluenceRange = NULL;
    BoundingBoxRenderContainer *mCurrentRenderContainer = NULL;
    QList<InfluenceRange*> mInfluenceRange;
};

#endif // BOUNDINGBOXRENDERCONTAINER_H
