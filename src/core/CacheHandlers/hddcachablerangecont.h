#ifndef HDDCACHABLERANGECONT_H
#define HDDCACHABLERANGECONT_H
#include "hddcachablecont.h"
#include "tmpfilehandlers.h"
#include "framerange.h"
class eTask;
class HDDCachableCacheHandler;

class HDDCachableRangeCont : public HDDCachable {
    friend class SoundContainerTmpFileDataSaver;
protected:
    HDDCachableRangeCont(const FrameRange &range,
                         HDDCachableCacheHandler * const parent) :
        mRange(range), mParentCacheHandler_k(parent) {}
    virtual int clearMemory() = 0;
public:
    void noDataLeft_k() final;

    int getRangeMin() const;
    const FrameRange& getRange() const {
        return mRange;
    }
    int getRangeMax() const;
    int rangeSpan() const {
        return mRange.span();
    }

    void setUnaryRange(const int unary);
    void setRangeMax(const int max);
    void setRangeMin(const int min);
    void setRange(const FrameRange &range);
    bool inRange(const int unary) const;
private:
    FrameRange mRange;
    HDDCachableCacheHandler * const mParentCacheHandler_k;
};

#endif // HDDCACHABLERANGECONT_H
