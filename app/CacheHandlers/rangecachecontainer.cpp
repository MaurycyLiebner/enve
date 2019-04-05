#include "rangecachecontainer.h"

int RangeCacheContainer::getRangeMin() const {
    return mRange.fMin;
}

int RangeCacheContainer::getRangeMax() const {
    return mRange.fMax;
}

bool RangeCacheContainer::inRange(const int &unary) const {
    return mRange.inRange(unary);
}

void RangeCacheContainer::setUnaryRange(const int &unary) {
    mRange.fMin = unary;
    mRange.fMax = unary;
}

void RangeCacheContainer::setRangeMax(const int &max) {
    mRange.fMax = max;
}

void RangeCacheContainer::setRangeMin(const int &min) {
    mRange.fMin = min;
}

void RangeCacheContainer::setRange(const FrameRange &range) {
    mRange = range;
}
