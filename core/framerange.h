#ifndef FRAMERANGE_H
#define FRAMERANGE_H
#include <QtCore>

//! @brief Inclusive frame range
struct FrameRange {
    int min; //! @brief Inclusive min frame
    int max; //! @brief Inclusive max frame

    bool singleFrame() const {
        return min == max;
    }

    bool isValid() const {
        return max < min;
    }

    bool contains(const int& frame) const {
        return frame >= min && frame <= max;
    }

    bool overlaps(const FrameRange& b) const {
        return !this->operator*(b).isValid();
    }

    void fixOrder() {
        if(min <= max) return;
        auto maxT = max;
        max = min;
        min = maxT;
    }

    FrameRange operator*(const FrameRange& b) const {
        return {qMax(this->min, b.min), qMin(this->max, b.max)};
    }
    FrameRange& operator*=(const FrameRange& b) {
        this->min = qMax(this->min, b.min);
        this->max = qMin(this->max, b.max);
        return *this;
    }
};

#endif // FRAMERANGE_H
