#ifndef HDDCACHABLECACHEHANDLER_H
#define HDDCACHABLECACHEHANDLER_H
#include "minimalcachehandler.h"
#include "hddcachablerangecont.h"
#include "rangeset.h"

class HDDCachableCacheHandler :
        public RangeSet<HDDCachableRangeCont,
                        &HDDCachableRangeCont::getRange,
                        stdsptr<HDDCachableRangeCont>> {
public:
    typedef HDDCachableRangeCont Cont;
    typedef QList<stdsptr<Cont>>::const_iterator ContCIter;
    typedef QList<stdsptr<Cont>>::iterator ContIter;

    int getFirstEmptyOrCachedFrameAfterFrame(
            const int frame, Cont **contP = nullptr) const {
        int currFrame = frame + 1;
        Cont *cont = nullptr;
        while(true) {
            cont = atFrame(currFrame);
            if(!cont) break;
            if(!cont->storesDataInMemory()) break;
            currFrame = cont->getRangeMax() + 1;
        }
        if(contP) *contP = cont;
        return currFrame;
    }

    void blockConts(const FrameRange &range, const bool blocked);

    void cacheDataBeforeRelFrame(const int relFrame);
    void cacheDataAfterRelFrame(const int relFrame);

    void cacheFirstContainer();
    void cacheLastContainer();

    void drawCacheOnTimeline(QPainter * const p,
                             const QRectF &drawRect,
                             const int startFrame,
                             const int endFrame,
                             const qreal unit = 1) const;
};

#endif // HDDCACHABLECACHEHANDLER_H
