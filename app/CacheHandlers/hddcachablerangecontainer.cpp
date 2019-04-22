#include "hddcachablerangecontainer.h"
#include "hddcachablecachehandler.h"

void HDDCachableRangeContainer::noDataLeft_k() {
    const auto thisRef = ref<HDDCachableRangeContainer>();
    mParentCacheHandler_k->removeRenderContainer(thisRef);
}

int HDDCachableRangeContainer::getRangeMin() const {
    return mRange.fMin;
}

int HDDCachableRangeContainer::getRangeMax() const {
    return mRange.fMax;
}

bool HDDCachableRangeContainer::inRange(const int &unary) const {
    return mRange.inRange(unary);
}

void HDDCachableRangeContainer::setUnaryRange(const int &unary) {
    mRange.fMin = unary;
    mRange.fMax = unary;
}

void HDDCachableRangeContainer::setRangeMax(const int &max) {
    mRange.fMax = max;
}

void HDDCachableRangeContainer::setRangeMin(const int &min) {
    mRange.fMin = min;
}

void HDDCachableRangeContainer::setRange(const FrameRange &range) {
    mRange = range;
}
