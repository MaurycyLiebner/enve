#ifndef HDDCACHABLECACHEHANDLER_H
#define HDDCACHABLECACHEHANDLER_H
#include "minimalcachehandler.h"
template <typename T>
class HDDCachableRangeContainer;

template <typename T>
class HDDCachableCacheHandler : public MinimalCacheHandler<T> {
    static_assert(std::is_base_of<HDDCachableRangeContainer<T>, T>::value,
                  "HDDCachableCacheHandler can be used only with HDDCachableRangeContainer derived classes");

public:
    int getFirstEmptyOrCachedFrameAfterFrame(
            const int &frame,
            T **contP = nullptr) const {
        int currFrame = frame + 1;
        T *cont = nullptr;
        while(true) {
            cont = this->getRenderContainerAtRelFrame(currFrame);
            if(!cont) break;
            if(!cont->storesDataInMemory()) break;
            currFrame = cont->getRangeMax() + 1;
        }
        if(contP) *contP = cont;
        return currFrame;
    }

    int getNumberNotCachedBeforeRelFrame(const int &relFrame) const {
        int nNotCached = 0;
        int firstId = this->getRenderContainerIdAtOrBeforeRelFrame(relFrame - 1);
        for(int i = 0; i < firstId; i++) {
            if(this->mRenderContainers.at(i)->storesDataInMemory()) {
                nNotCached++;
            }
        }
        return nNotCached;
    }
};

#endif // HDDCACHABLECACHEHANDLER_H
