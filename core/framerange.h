#ifndef FRAMERANGE_H
#define FRAMERANGE_H
#include <QtCore>
#define TEN_MIL 10000000
//! @brief Inclusive range
struct iValueRange {
    static int EMIN;
    static int EMAX;

    int fMin; //! @brief Inclusive range min
    int fMax; //! @brief Inclusive range max

    bool isUnary() const {
        return fMin == fMax;
    }

    bool isValid() const {
        return fMax >= fMin;
    }

    bool inRange(const int val) const {
        return val >= fMin && val <= fMax;
    }

    bool inRange(const iValueRange& range) const {
        return inRange(range.fMin) && inRange(range.fMax);
    }

    int span() const {
        return qMax(0, fMax - fMin + 1);
    }

    iValueRange shifted(const int by) const {
        return {fMin + by, fMax + by};
    }

    bool overlaps(const iValueRange& b) const {
        return this->operator*(b).isValid();
    }

    bool neighbours(const iValueRange& b) const {
        if(b.fMax == fMin - 1) return true;
        if(fMax == b.fMin - 1) return true;
        return false;
    }

    void fixOrder() {
        if(fMin <= fMax) return;
        auto maxT = fMax;
        fMax = fMin;
        fMin = maxT;
    }

    bool operator<(const iValueRange& b) const {
        if(overlaps(b)) return false;
        return fMin < b.fMin;
    }

    bool operator>(const iValueRange& b) const {
        if(overlaps(b)) return false;
        return fMin > b.fMin;
    }

    bool operator==(const iValueRange& b) const {
        return fMin == b.fMin && fMax == b.fMax;
    }

    bool operator!=(const iValueRange& b) const {
        return fMin != b.fMin || fMax != b.fMax;
    }

    iValueRange operator*(const iValueRange& b) const {
        return {qMax(this->fMin, b.fMin), qMin(this->fMax, b.fMax)};
    }

    iValueRange& operator*=(const iValueRange& b) {
        this->fMin = qMax(this->fMin, b.fMin);
        this->fMax = qMin(this->fMax, b.fMax);
        return *this;
    }

    iValueRange operator+(const iValueRange& b) const {
        return {qMin(this->fMin, b.fMin), qMax(this->fMax, b.fMax)};
    }
    iValueRange& operator+=(const iValueRange& b) {
        this->fMin = qMin(this->fMin, b.fMin);
        this->fMax = qMax(this->fMax, b.fMax);
        return *this;
    }
};

struct qValueRange {
    qreal fMin;
    qreal fMax;

    bool isValid() const {
        return fMax > fMin;
    }

    bool inRange(const qreal val) const {
        return val >= fMin && val <= fMax;
    }

    qreal span() const {
        return qMax(0., fMax - fMin);
    }

    qValueRange shifted(const qreal by) const {
        return {fMin + by, fMax + by};
    }

    bool overlaps(const qValueRange& b) const {
        return !this->operator*(b).isValid();
    }

    void fixOrder() {
        if(fMin <= fMax) return;
        auto maxT = fMax;
        fMax = fMin;
        fMin = maxT;
    }

    qValueRange operator/(const qreal b) const {
        return {this->fMin/b, this->fMax/b};
    }

    qValueRange operator*(const qreal b) const {
        return {this->fMin*b, this->fMax*b};
    }

    qValueRange operator*(const qValueRange& b) const {
        return {qMax(this->fMin, b.fMin), qMin(this->fMax, b.fMax)};
    }

    qValueRange& operator*=(const qValueRange& b) {
        this->fMin = qMax(this->fMin, b.fMin);
        this->fMax = qMin(this->fMax, b.fMax);
        return *this;
    }

    qValueRange operator+(const qValueRange& b) const {
        return {qMin(this->fMin, b.fMin), qMax(this->fMax, b.fMax)};
    }

    qValueRange& operator+=(const qValueRange& b) {
        this->fMin = qMin(this->fMin, b.fMin);
        this->fMax = qMax(this->fMax, b.fMax);
        return *this;
    }
};

//! @brief Inclusive frame range
typedef iValueRange FrameRange;
typedef iValueRange AbsFrameRange;
typedef iValueRange RelFrameRange;

typedef iValueRange IdRange;

//! @brief Inclusive sample range
typedef iValueRange SampleRange;
#endif // FRAMERANGE_H
