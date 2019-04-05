#ifndef HDDCACHABLECACHEHANDLER_H
#define HDDCACHABLECACHEHANDLER_H
#include "minimalcachehandler.h"
template <typename T>
class HDDCachableRangeContainer;

template <typename T>
class HDDCachableCacheHandler : public RangeCacheHandler {
public:
    int getFirstEmptyOrCachedFrameAfterFrame(
            const int &frame,
            T **contP = nullptr) const {
        int currFrame = frame + 1;
        T *cont = nullptr;
        while(true) {
            cont = this->getRenderContainerAtRelFrame<T>(currFrame);
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
