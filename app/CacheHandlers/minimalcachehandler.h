#ifndef MINIMALCACHEHANDLER_H
#define MINIMALCACHEHANDLER_H
#include <QList>
#include <QPainter>
#include "framerange.h"
#include "smartPointers/stdselfref.h"
#include "smartPointers/sharedpointerdefs.h"
#include "memoryhandler.h"
#include "global.h"
class RangeCacheContainer;

class RangeCacheHandler {
    typedef RangeCacheContainer RCC;
public:
    void removeRenderContainer(const stdsptr<RCC>& cont);

    int getRenderContainterInsertIdAtRelFrame(
                        const int &relFrame) const;

    bool getRenderContainterIdAtRelFrame(const int &relFrame,
                                         int *id) const;

    int getFirstEmptyFrameAfterFrame(const int &frame) const;

    int getFirstEmptyFrameAtOrAfterFrame(const int &frame) const;

    void setContainersInFrameRangeBlocked(const FrameRange &range,
                                          const bool &blocked);

    void clearCache();

    void cacheDataBeforeRelFrame(const int &relFrame);

    void cacheDataAfterRelFrame(const int &relFrame);

    void cacheFirstContainer();

    void cacheLastContainer();

    int getContainerCountAfterRelFrame(const int &relFrame) const;


    void updateAllAfterFrameInMemoryHandler(const int &relFrame);

    template <typename T = RCC>
    T *getRenderContainerAtRelFrame(const int &frame) const {
        int id;
        if(getRenderContainterIdAtRelFrame(frame, &id)) {
            return static_cast<T*>(mRenderContainers.at(id).get());
        }
        return nullptr;
    }

    int getRenderContainerIdAtOrBeforeRelFrame(const int &frame) const;
    template <typename T = RCC>
    T *getRenderContainerAtOrBeforeRelFrame(const int &frame) const {
        T *cont = getRenderContainerAtRelFrame<T>(frame);
        if(!cont) {
            int id = getRenderContainterInsertIdAtRelFrame(frame) - 1;
            if(id >= 0 && id < mRenderContainers.length()) {
                cont = static_cast<T*>(mRenderContainers.at(id).get());
            }
        }
        return cont;
    }


    int getRenderContainerIdAtOrAfterRelFrame(const int &frame) const;

    template <typename T = RCC>
    T *getRenderContainerAtOrAfterRelFrame(const int &frame) const {
        T *cont = getRenderContainerAtRelFrame<T>(frame);
        if(!cont) {
            int id = getRenderContainterInsertIdAtRelFrame(frame);
            if(id >= 0 && id < mRenderContainers.length()) {
                cont = static_cast<T*>(mRenderContainers.at(id).get());
            }
        }
        return cont;
    }


    void drawCacheOnTimeline(QPainter * const p,
                             const QRect drawRect,
                             const int &startFrame,
                             const int &endFrame) const;

    void clearCacheForRelFrameRange(const FrameRange& range);

    template<typename T, typename... Args>
    T *createNewRenderContainerAtRelFrame(
            const FrameRange &frameRange, Args && ...args) {
        static_assert(std::is_base_of<RCC, T>::value,
                      "MinimalCacheHandler can be used only with "
                      "MinimalCacheContainer derived classes");
        auto cont = SPtrCreateTemplated(T)(args..., frameRange, this);
        int contId = getRenderContainterInsertIdAtRelFrame(frameRange.fMin);
        mRenderContainers.insert(contId, cont);
        return cont.get();
    }

    template<typename T, typename... Args>
    T *createNewRenderContainerAtRelFrame(
            const int &relFrame, Args && ...args) {
        return createNewRenderContainerAtRelFrame<T>(
                {relFrame, relFrame}, args...);
    }

    QList<FrameRange> getMissingRanges(const FrameRange& range) const;
protected:
    IdRange rangeToListIdRange(const FrameRange &range);

    QList<stdsptr<RCC>> mRenderContainers;
};

#endif // MINIMALCACHEHANDLER_H
