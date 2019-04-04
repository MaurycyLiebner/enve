#ifndef MINIMALCACHEHANDLER_H
#define MINIMALCACHEHANDLER_H
#include <QList>
#include <QPainter>
#include "framerange.h"
#include "smartPointers/stdselfref.h"
#include "smartPointers/sharedpointerdefs.h"
#include "memoryhandler.h"
#include "global.h"

template <typename T>
class MinimalCacheHandler {
    static_assert(std::is_base_of<MinimalCacheContainer, T>::value,
                  "MinimalCacheHandler can be used only with MinimalCacheContainer derived classes");
public:
    void removeRenderContainer(const stdsptr<T>& cont) {
        mRenderContainers.removeOne(cont);
    }

    int getRenderContainterInsertIdAtRelFrame(
                        const int &relFrame) const {
        int minId = 0;
        int maxId = mRenderContainers.count();

        while(minId < maxId) {
            int guess = (minId + maxId)/2;
            auto cont = mRenderContainers.at(guess).get();
            int contFrame = cont->getRangeMin();
            Q_ASSERT(contFrame != relFrame);
            if(contFrame > relFrame) {
                if(guess == maxId) return minId;
                maxId = guess;
            } else if(contFrame < relFrame) {
                if(guess == minId) return maxId;
                minId = guess;
            }
        }
        return 0;
    }

    bool getRenderContainterIdAtRelFrame(const int &relFrame,
                                         int *id) const {
        int minId = 0;
        int maxId = mRenderContainers.count() - 1;

        while(minId <= maxId) {
            int guess = (minId + maxId)/2;
            auto cont = mRenderContainers.at(guess).get();
            if(cont->inRange(relFrame)) {
                *id = guess;
                return true;
            }
            int contFrame = cont->getRangeMin();
            if(contFrame > relFrame) {
                if(maxId == guess) {
                    *id = minId;
                    return mRenderContainers.at(minId)->inRange(relFrame);
                } else {
                    maxId = guess;
                }
            } else if(contFrame < relFrame) {
                if(minId == guess) {
                    *id = maxId;
                    return mRenderContainers.at(maxId)->inRange(relFrame);
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

    int getFirstEmptyFrameAfterFrame(const int &frame) const {
        int currFrame = frame + 1;
        T *cont = nullptr;
        while(true) {
            cont = getRenderContainerAtRelFrame(currFrame);
            if(!cont) return currFrame;
            currFrame = cont->getMaxRelFrame() + 1;
        }
    }

    int getFirstEmptyFrameAtOrAfterFrame(const int &frame) const {
        int currFrame = frame;
        T *cont = nullptr;
        while(true) {
            cont = getRenderContainerAtRelFrame(currFrame);
            if(!cont) return currFrame;
            currFrame = cont->getMaxRelFrame() + 1;
        }
    }

    void setContainersInFrameRangeBlocked(const FrameRange &range,
                                          const bool &blocked) {
        IdRange idRange = rangeToListIdRange(range);
        for(int i = idRange.fMin; i <= idRange.fMax; i++) {
            mRenderContainers.at(i)->setBlocked(blocked);
        }
    }

    void clearCache() {
        mRenderContainers.clear();
    }

    void cacheDataBeforeRelFrame(const int &relFrame) {
        int lastId = getRenderContainerIdAtOrBeforeRelFrame(relFrame);
        for(int i = 0; i < lastId; i++) {
            mRenderContainers.at(i)->cacheFreeAndRemoveFromMemoryHandler();
        }
    }

    void cacheDataAfterRelFrame(const int &relFrame) {
        int firstId = getRenderContainerIdAtOrAfterRelFrame(relFrame);
        for(int i = firstId; i < mRenderContainers.count(); i++) {
            mRenderContainers.at(i)->cacheFreeAndRemoveFromMemoryHandler();
        }
    }

    void cacheFirstContainer() {
        if(mRenderContainers.isEmpty()) return;
        mRenderContainers.first()->cacheFreeAndRemoveFromMemoryHandler();
    }

    void cacheLastContainer() {
        if(mRenderContainers.isEmpty()) return;
        mRenderContainers.last()->cacheFreeAndRemoveFromMemoryHandler();
    }

    int getContainerCountAfterRelFrame(const int &relFrame) const {
        int firstId = getRenderContainerIdAtOrAfterRelFrame(relFrame + 1);
        return mRenderContainers.count() - firstId;
    }


    void updateAllAfterFrameInMemoryHandler(const int &relFrame) {
        int firstId = getRenderContainerIdAtOrAfterRelFrame(relFrame + 1);
        for(int i = mRenderContainers.count() - 1; i >= firstId; i--) {
            T *cont = mRenderContainers.at(i).get();
            if(cont->handledByMemoryHandler()) {
                MemoryHandler::sGetInstance()->containerUpdated(cont);
            }
        }
    }

    T *getRenderContainerAtRelFrame(const int &frame) const {
        int id;
        if(getRenderContainterIdAtRelFrame(frame, &id)) {
            return mRenderContainers.at(id).get();
        }
        return nullptr;
    }

    int getRenderContainerIdAtOrBeforeRelFrame(const int &frame) const {
        int id;
        if(!getRenderContainterIdAtRelFrame(frame, &id)) {
            id = getRenderContainterInsertIdAtRelFrame(frame) - 1;
        }
        return id;
    }

    T *getRenderContainerAtOrBeforeRelFrame(const int &frame) const {
        T *cont = getRenderContainerAtRelFrame(frame);
        if(!cont) {
            int id = getRenderContainterInsertIdAtRelFrame(frame) - 1;
            if(id >= 0 && id < mRenderContainers.length()) {
                cont = mRenderContainers.at(id).get();
            }
        }
        return cont;
    }


    int getRenderContainerIdAtOrAfterRelFrame(const int &frame) const {
        int id;
        if(!getRenderContainterIdAtRelFrame(frame, &id)) {
            id = getRenderContainterInsertIdAtRelFrame(frame);
        }
        return id;
    }

    T *getRenderContainerAtOrAfterRelFrame(const int &frame) const {
        T *cont = getRenderContainerAtRelFrame(frame);
        if(!cont) {
            int id = getRenderContainterInsertIdAtRelFrame(frame);
            if(id >= 0 && id < mRenderContainers.length()) {
                cont = mRenderContainers.at(id).get();
            }
        }
        return cont;
    }

    void drawCacheOnTimeline(QPainter * const p,
                             const QRect drawRect,
                             const int &startFrame,
                             const int &endFrame) const {
        if(startFrame > endFrame) return;
        p->setBrush(QColor(0, 255, 0, 75));
        p->setPen(Qt::NoPen);
        const qreal pixelsPerFrame = static_cast<qreal>(drawRect.width())/
                (endFrame - startFrame + 1);
        int lastDrawnFrame = startFrame;
        int lastDrawX = 0;
        bool lastStoresInMemory = true;
        for(const auto &cont : mRenderContainers) {
            int afterMaxFrame = cont->getRangeMax() + 1;
            if(afterMaxFrame < startFrame) continue;
            int minFrame = cont->getRangeMin();
            if(minFrame > endFrame + 1) return;

            if(afterMaxFrame > endFrame) afterMaxFrame = endFrame + 1;
            if(minFrame < startFrame) minFrame = startFrame;

            int dFrame = minFrame - startFrame;
            int xT = qRound(dFrame*pixelsPerFrame);

            int widthT = qRound(pixelsPerFrame*(afterMaxFrame - minFrame));
            if(lastDrawnFrame == minFrame) {
                widthT += xT - lastDrawX;
                xT = lastDrawX;
            }
            bool storesInMemory = cont->storesDataInMemory();
            if(storesInMemory != lastStoresInMemory) {
                if(storesInMemory) {
                    p->setBrush(QColor(0, 255, 0, 75));
                } else {
                    p->setBrush(QColor(0, 0, 255, 75));
                }
                lastStoresInMemory = storesInMemory;
            }

            p->drawRect(xT, drawRect.top(), widthT, drawRect.height());
            lastDrawnFrame = afterMaxFrame;
            lastDrawX = xT + widthT;
        }
    }

    void clearCacheForRelFrameRange(const FrameRange& range) {
        int minId;
        if(!getRenderContainterIdAtRelFrame(range.fMin, &minId)) {
            minId = getRenderContainterInsertIdAtRelFrame(range.fMin);
        }
        int maxId;
        if(!getRenderContainterIdAtRelFrame(range.fMax, &maxId)) {
            maxId = getRenderContainterInsertIdAtRelFrame(range.fMax) - 1;
        }
        for(int i = minId; i <= maxId; i++) {
            mRenderContainers.removeAt(minId);
        }
    }

    template<typename... Args>
    T *createNewRenderContainerAtRelFrame(
            const FrameRange &frameRange, Args && ...args) {
        auto cont = SPtrCreateTemplated(T)(args..., frameRange, this);
        int contId = getRenderContainterInsertIdAtRelFrame(frameRange.fMin);
        mRenderContainers.insert(contId, cont);
        return cont.get();
    }

    template<typename... Args>
    T *createNewRenderContainerAtRelFrame(
            const int &relFrame, Args && ...args) {
        return createNewRenderContainerAtRelFrame(
                {relFrame, relFrame}, args...);
    }

    QList<FrameRange> getMissingRanges(
            const FrameRange& range) const {
        QList<FrameRange> result;
        int currentFrame = range.fMin;
        while(currentFrame <= range.fMax) {
            auto cont = getRenderContainerAtOrAfterRelFrame(currentFrame);
            if(!cont) {
                result.append({currentFrame, range.fMax});
                break;
            }
            auto contRange = cont->getRange();
            if(!contRange.inRange(currentFrame)) {
                result.append({currentFrame,
                               qMin(range.fMax, contRange.fMin - 1)});
            }
            currentFrame = contRange.fMax + 1;
        }

        return result;
    }
protected:
    IdRange rangeToListIdRange(const FrameRange &range) {
        int minId;
        if(!getRenderContainterIdAtRelFrame(range.fMin, &minId)) {
            minId = getRenderContainterInsertIdAtRelFrame(range.fMin);
        }
        int maxId;
        if(!getRenderContainterIdAtRelFrame(range.fMax, &maxId)) {
            maxId = getRenderContainterInsertIdAtRelFrame(range.fMax) - 1;
        }
        return {minId, maxId};
    }

    QList<stdsptr<T>> mRenderContainers;
};

#endif // MINIMALCACHEHANDLER_H
