#ifndef RANGECACHECONTAINER_H
#define RANGECACHECONTAINER_H
#include "framerange.h"
#include "minimalcachecontainer.h"

class RangeCacheContainer : public MinimalCacheContainer {
public:
    const int &getRangeMin() const;
    const FrameRange& getRange() const {
        return mRange;
    }
    const int &getRangeMax() const;
    int rangeSpan() const {
        return mRange.span();
    }

    void setUnaryRange(const int &unary);
    void setRangeMax(const int &max);
    void setRangeMin(const int &min);
    void setRange(const FrameRange &range);
    bool inRange(const int &unary) const;
protected:
    RangeCacheContainer() {}
    RangeCacheContainer(const FrameRange &range) :
        mRange(range) {}
private:
    FrameRange mRange{0, 0};
};

#endif // RANGECACHECONTAINER_H
