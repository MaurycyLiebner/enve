#ifndef RANGEMAP_H
#define RANGEMAP_H
#include "exceptions.h"
#include "framerange.h"

template <class T>
class RangeMap : public std::map<iValueRange, T> {
    typedef typename std::map<iValueRange, T>::iterator iterator;
    typedef typename std::map<iValueRange, T>::const_iterator const_iterator;
public:
    //! @brief Returns iterator range [first, second)
    std::pair<iterator, iterator> range(const iValueRange &range) {
        return {atOrAfterFrame(range.fMin), afterFrame(range.fMax)};
    }

    //! @brief Returns iterator range [first, second)
    std::pair<const_iterator, const_iterator> range(const iValueRange &range) const {
        return {atOrAfterFrame(range.fMin), afterFrame(range.fMax)};
    }

    int count() const { return this->size(); }

    iValueRange firstEmptyRangeUpperBound(const int keyMin) const {
        return firstEmptyRangeLowerBound(keyMin + 1);
    }

    iValueRange firstEmptyRangeLowerBound(const int keyMin) const {
        int currKeyMin = keyMin;
        while(true) {
            const auto it = atFrame(currKeyMin);
            if(it == this->end()) {
                const auto after = afterFrame(currKeyMin);
                const int lastEmpty = after == this->end() ?
                            iValueRange::EMAX :
                            (it->first.fMin - 1);
                return {currKeyMin, lastEmpty};
            }
            currKeyMin = it->first.fMax + 1;
        }
    }

    QList<iValueRange> getMissingRanges(const iValueRange &range) const {
        QList<iValueRange> result;

        int currKeyMin = range.fMin;
        while(currKeyMin < range.fMax) {
            const auto empty = firstEmptyRangeLowerBound(currKeyMin);
            result << empty;
            currKeyMin = empty.fMax + 1;
        }

        return result;
    }

    bool isEmpty() const { return this->empty(); }

    iterator atFrame(const int frame) {
        return this->find({frame, frame});
    }

    const_iterator atFrame(const int frame) const {
        return this->find({frame, frame});
    }

    iterator atOrBeforeFrame(const int frame) {
        auto nextIt = afterFrame(frame);
        if(nextIt == this->begin()) return this->end();
        return --nextIt;
    }

    const_iterator atOrBeforeFrame(const int frame) const {
        auto nextIt = afterFrame(frame);
        if(nextIt == this->begin()) return this->end();
        return --nextIt;
    }

    iterator atOrAfterFrame(const int frame) {
        return this->lower_bound({frame, frame});
    }

    const_iterator atOrAfterFrame(const int frame) const {
        return this->lower_bound({frame, frame});
    }

    iterator beforeFrame(const int frame) {
        auto nextIt = atOrAfterFrame(frame);
        if(nextIt == this->begin()) return this->end();
        return --nextIt;
    }

    const_iterator beforeFrame(const int frame) const {
        auto nextIt = atOrAfterFrame(frame);
        if(nextIt == this->begin()) return this->end();
        return --nextIt;
    }

    iterator afterFrame(const int frame) {
        return this->upper_bound({frame, frame});
    }

    const_iterator afterFrame(const int frame) const {
        return this->upper_bound({frame, frame});
    }
};

#endif // RANGEMAP_H
