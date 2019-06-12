#ifndef RANGESET_H
#define RANGESET_H
#include "atomicset.h"

template <class T, const iValueRange& (T::*RangeGetter)() const,
          class TCont = RawVal<T>>
class RangeSet {
    typedef typename std::map<iValueRange, TCont>::iterator iterator;
    typedef typename std::map<iValueRange, TCont>::const_iterator const_iterator;
public:
    void remove(const TCont& element) {
        remove(((*element).*RangeGetter)());
    }

    void remove(const iterator& begin, const iterator& end) {
        mList.erase(begin, end);
    }

    void remove(const iValueRange &range) {
        mList.erase(range);
    }

    //! @brief Returns iterator range [first, second)
    std::pair<iterator, iterator> itRange(const iValueRange &range) {
        return {itAtOrAfterFrame(range.fMin),
                itAfterFrame(range.fMax)};
    }

    //! @brief Returns iterator range [first, second)
    std::pair<const_iterator, const_iterator>
        itRange(const iValueRange &range) const {
        return {itAtOrAfterFrame(range.fMin),
                itAfterFrame(range.fMax)};
    }

    void clear() { mList.clear(); }

    int count() const { return mList.size(); }

    iValueRange firstEmptyRangeUpperBound(const int keyMin) const {
        return firstEmptyRangeLowerBound(keyMin + 1);
    }

    iValueRange firstEmptyRangeLowerBound(const int keyMin) const {
        int currKeyMin = keyMin;
        while(true) {
            auto cont = itAtFrame(currKeyMin);
            if(cont == mList.end()) {
                const auto after = itAfterFrame(currKeyMin);
                const int lastEmpty = after == mList.end() ?
                            iValueRange::EMAX :
                            (extractElement<T>(cont)->*RangeGetter)().fMin - 1;
                return {currKeyMin, lastEmpty};
            }
            currKeyMin = (extractElement<T>(cont)->*RangeGetter)().fMax + 1;
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

    bool isEmpty() const { return mList.empty(); }

    void add(const TCont& element) {
        const auto ret = mList.insert({((*element).*RangeGetter)(), element});
        if(!ret.second) RuntimeThrow("Range already occupied by a different element");
    }

    template <class U = T>
    inline U* extractElement(const iterator it) {
        if(it == mList.end()) return nullptr;
        return static_cast<U*>(&*it->second);
    }

    template <class U = T>
    inline const U* extractElement(const const_iterator it) const {
        if(it == mList.end()) return nullptr;
        return static_cast<const U*>(&*it->second);
    }

    template <class U = T>
    U* first() {
        return extractElement<U>(mList.begin());
    }

    template <class U = T>
    const U* first() const {
        return extractElement<U>(mList.begin());
    }

    template <class U = T>
    U* last() {
        return extractElement<U>(mList.rbegin());
    }

    template <class U = T>
    const U* last() const {
        return extractElement<U>(mList.rbegin());
    }

    template <class U = T>
    U* atFrame(const int frame) {
        return extractElement<U>(itAtFrame(frame));
    }

    template <class U = T>
    const U* atFrame(const int frame) const {
        return extractElement<U>(itAtFrame(frame));
    }

    template <class U = T>
    U* atOrBeforeFrame(const int frame) {
        return extractElement<U>(itAtOrBeforeFrame(frame));
    }

    template <class U = T>
    const U* atOrBeforeFrame(const int frame) const {
        return extractElement<U>(itAtOrBeforeFrame(frame));
    }

    template <class U = T>
    U* atOrAfterFrame(const int frame) {
        return extractElement<U>(itAtOrAfterFrame(frame));
    }

    template <class U = T>
    const U* atOrAfterFrame(const int frame) const {
        return extractElement<U>(itAtOrAfterFrame(frame));
    }

    template <class U = T>
    U* beforeFrame(const int frame) {
        return extractElement<U>(itBeforeFrame(frame));
    }

    template <class U = T>
    const U* beforeFrame(const int frame) const {
        return extractElement<U>(itBeforeFrame(frame));
    }

    template <class U = T>
    U* afterFrame(const int frame) {
        return extractElement<U>(itAfterFrame(frame));
    }

    template <class U = T>
    const U* afterFrame(const int frame) const {
        return extractElement<U>(itAfterFrame(frame));
    }

    iterator begin() { return mList.begin(); }
    const_iterator begin() const { return mList.begin(); }

    iterator end() { return mList.end(); }
    const_iterator end() const { return mList.end(); }

    iterator itAtFrame(const int frame) {
        return mList.find({frame, frame});
    }

    const_iterator itAtFrame(const int frame) const {
        return mList.find({frame, frame});
    }

    iterator itAtOrBeforeFrame(const int frame) {
        auto nextIt = itAfterFrame(frame);
        if(nextIt == mList.begin()) return mList.end();
        return --nextIt;
    }

    const_iterator itAtOrBeforeFrame(const int frame) const {
        auto nextIt = itAfterFrame(frame);
        if(nextIt == mList.begin()) return mList.end();
        return --nextIt;
    }

    iterator itAtOrAfterFrame(const int frame) {
        return mList.lower_bound({frame, frame});
    }

    const_iterator itAtOrAfterFrame(const int frame) const {
        return mList.lower_bound({frame, frame});
    }

    iterator itBeforeFrame(const int frame) {
        auto nextIt = itAtOrAfterFrame(frame);
        if(nextIt == mList.begin()) return mList.end();
        return --nextIt;
    }

    const_iterator itBeforeFrame(const int frame) const {
        auto nextIt = itAtOrAfterFrame(frame);
        if(nextIt == mList.begin()) return mList.end();
        return --nextIt;
    }

    iterator itAfterFrame(const int frame) {
        return mList.upper_bound({frame, frame});
    }

    const_iterator itAfterFrame(const int frame) const {
        return mList.upper_bound({frame, frame});
    }

private:
    std::map<iValueRange, TCont> mList;
};

#endif // RANGESET_H
