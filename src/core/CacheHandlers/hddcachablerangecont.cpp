#include "hddcachablerangecont.h"
#include "hddcachablecachehandler.h"

void HddCachableRangeCont::noDataLeft_k() {
    if(!mParentCacheHandler_k) return;
    const auto thisRef = ref<HddCachableRangeCont>();
    mParentCacheHandler_k->remove(thisRef);
}

int HddCachableRangeCont::getRangeMin() const {
    return mRange.fMin;
}

int HddCachableRangeCont::getRangeMax() const {
    return mRange.fMax;
}

bool HddCachableRangeCont::inRange(const int unary) const {
    return mRange.inRange(unary);
}

void HddCachableRangeCont::setUnaryRange(const int unary) {
    mRange.fMin = unary;
    mRange.fMax = unary;
}

void HddCachableRangeCont::setRangeMax(const int max) {
    mRange.fMax = max;
}

void HddCachableRangeCont::setRangeMin(const int min) {
    mRange.fMin = min;
}

void HddCachableRangeCont::setRange(const FrameRange &range) {
    mRange = range;
}
