#ifndef HDDCACHABLECACHEHANDLER_H
#define HDDCACHABLECACHEHANDLER_H
#include "minimalcachehandler.h"
#include "hddcachablerangecontainer.h"

class HDDCachableCacheHandler {
    typedef HDDCachableRangeContainer Cont;
public:
    int getFirstEmptyOrCachedFrameAfterFrame(
            const int &frame, Cont **contP = nullptr) const {
        int currFrame = frame + 1;
        Cont *cont = nullptr;
        while(true) {
            cont = this->atRelFrame<Cont>(currFrame);
            if(!cont) break;
            if(!cont->storesDataInMemory()) break;
            currFrame = cont->getRangeMax() + 1;
        }
        if(contP) *contP = cont;
        return currFrame;
    }

    void removeRenderContainer(const stdsptr<Cont>& cont);

    int insertIdForRelFrame(const int &relFrame) const;

    bool idAtRelFrame(const int &relFrame, int *id) const;

    int getFirstEmptyFrameAfterFrame(const int &frame) const;

    int firstEmptyFrameAtOrAfterFrame(const int &frame) const;

    void blockConts(const FrameRange &range, const bool &blocked);

    void clearCache();

    void cacheDataBeforeRelFrame(const int &relFrame);

    void cacheDataAfterRelFrame(const int &relFrame);

    void cacheFirstContainer();

    void cacheLastContainer();

    int countAfterRelFrame(const int &relFrame) const;

    template <typename T = Cont>
    T *atRelFrame(const int &frame) const {
        int id;
        if(idAtRelFrame(frame, &id)) {
            return static_cast<T*>(mRenderContainers.at(id).get());
        }
        return nullptr;
    }

    int idAtOrBeforeRelFrame(const int &frame) const;
    template <typename T = Cont>
    T *atOrBeforeRelFrame(const int &frame) const {
        T *cont = atRelFrame<T>(frame);
        if(!cont) {
            int id = insertIdForRelFrame(frame) - 1;
            if(id >= 0 && id < mRenderContainers.length()) {
                cont = static_cast<T*>(mRenderContainers.at(id).get());
            }
        }
        return cont;
    }


    int idAtOrAfterRelFrame(const int &frame) const;

    template <typename T = Cont>
    T *atOrAfterRelFrame(const int &frame) const {
        T *cont = atRelFrame<T>(frame);
        if(!cont) {
            int id = insertIdForRelFrame(frame);
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

    void clearRelRange(const FrameRange& range);

    template<typename T, typename... Args>
    T *createNew(const FrameRange &frameRange, Args && ...args) {
        static_assert(std::is_base_of<Cont, T>::value,
                      "MinimalCacheHandler can be used only with "
                      "MinimalCacheContainer derived classes");
        const auto cont = SPtrCreateTemplated(T)(args..., frameRange, this);
        const int contId = insertIdForRelFrame(frameRange.fMin);
        mRenderContainers.insert(contId, cont);
        return cont.get();
    }

    template<typename T, typename... Args>
    T *createNew(const int &relFrame, Args && ...args) {
        return createNew<T>({relFrame, relFrame}, args...);
    }

    QList<FrameRange> getMissingRanges(const FrameRange& range) const;
protected:
    IdRange rangeToListIdRange(const FrameRange &range);

    QList<stdsptr<Cont>> mRenderContainers;
};

#endif // HDDCACHABLECACHEHANDLER_H
