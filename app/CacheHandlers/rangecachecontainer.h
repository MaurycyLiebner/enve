#ifndef RANGECACHECONTAINER_H
#define RANGECACHECONTAINER_H
#include "framerange.h"
#include "minimalcachecontainer.h"

class RangeCacheContainer : public CacheContainer {
protected:
    RangeCacheContainer(const FrameRange &range) : mRange(range) {}
public:
    int getRangeMin() const;
    const FrameRange& getRange() const {
        return mRange;
    }
    int getRangeMax() const;
    int rangeSpan() const {
        return mRange.span();
    }

    void setUnaryRange(const int &unary);
    void setRangeMax(const int &max);
    void setRangeMin(const int &min);
    void setRange(const FrameRange &range);
    bool inRange(const int &unary) const;

    void setDataInMemory(const bool& dataInMemory) {
        mDataInMemory = dataInMemory;
    }

    bool storesDataInMemory() const {
        return mDataInMemory;
    }
private:
    FrameRange mRange;
    bool mDataInMemory = false;
};

#endif // RANGECACHECONTAINER_H
