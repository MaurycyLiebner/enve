#include "hddcachablerangecont.h"
#include "hddcachablecachehandler.h"

void HDDCachableRangeCont::noDataLeft_k() {
    const auto thisRef = ref<HDDCachableRangeCont>();
    mParentCacheHandler_k->remove(thisRef);
}

int HDDCachableRangeCont::getRangeMin() const {
    return mRange.fMin;
}

int HDDCachableRangeCont::getRangeMax() const {
    return mRange.fMax;
}

bool HDDCachableRangeCont::inRange(const int unary) const {
    return mRange.inRange(unary);
}

void HDDCachableRangeCont::setUnaryRange(const int unary) {
    mRange.fMin = unary;
    mRange.fMax = unary;
}

void HDDCachableRangeCont::setRangeMax(const int max) {
    mRange.fMax = max;
}

void HDDCachableRangeCont::setRangeMin(const int min) {
    mRange.fMin = min;
}

void HDDCachableRangeCont::setRange(const FrameRange &range) {
    mRange = range;
}
