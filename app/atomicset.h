#ifndef ATOMICSET_H
#define ATOMICSET_H

#include <QList>
#include "framerange.h"

template <class T>
struct RawVal {
    T fVal;

    T& operator*() const { return fVal; }
};

template <class T, const int& (T::*KeyGetter)() const,
          class TCont = RawVal<T>>
class AtomicSet {
    struct ContFrameLess {
        bool operator()(const TCont& element, const int relFrame) {
            return ((*element).*KeyGetter)() < relFrame;
        }
    };

    void removeId(const int id) {
        mList.removeAt(id);
    }
public:
    typedef typename QList<TCont>::iterator iterator;
    typedef typename QList<TCont>::const_iterator const_iterator;

    const_iterator lower_bound(const int frame) const {
        return std::lower_bound(mList.begin(), mList.end(),
                                frame, ContFrameLess());
    }

    iterator lower_bound(const int frame) {
        return std::lower_bound(mList.begin(), mList.end(),
                                frame, ContFrameLess());
    }

    inline iterator begin() { return mList.begin(); }
    inline iterator end() { return mList.end(); }

    inline const_iterator begin() const { return mList.begin(); }
    inline const_iterator end() const { return mList.end(); }

    bool remove(const TCont& element) {
        return mList.removeOne(element);
    }

    void remove(const iterator& begin, const iterator& end) {
        mList.erase(begin, end);
    }

    void remove(const FrameRange &range) {
        const auto idRange = rangeToIdRange(range);
        if(idRange.fMin == -1 || idRange.fMax == -1) return;

        for(int i = idRange.fMin; i <= idRange.fMax; i++)
            removeId(i);
    }

    void clear() { mList.clear(); }

    int count() const { return mList.count(); }

    int countAfterFrame(const int frame) const {
        const int firstId = idAtOrAfterFrame(frame + 1);
        return mList.count() - firstId;
    }

    int getFirstEmptyFrameAfterFrame(const int frame) const {
        int currFrame = frame + 1;
        T *cont = nullptr;
        while(true) {
            cont = atFrame(currFrame);
            if(!cont) return currFrame;
            currFrame = (cont->*KeyGetter)() + 1;
        }
    }

    int firstEmptyFrameAtOrAfterFrame(const int frame) const {
        int currFrame = frame;
        T *cont = nullptr;
        while(true) {
            cont = atFrame(currFrame);
            if(!cont) return currFrame;
            currFrame = (cont->*KeyGetter)() + 1;
        }
    }

    QList<FrameRange> getMissingRanges(const FrameRange &range) const {
        QList<FrameRange> result;
        int currentFrame = range.fMin;
        while(currentFrame <= range.fMax) {
            auto cont = atOrAfterFrame(currentFrame);
            if(!cont) {
                result.append({currentFrame, range.fMax});
                break;
            }
            const auto contRange = FrameRange{(cont->*KeyGetter)(),
                                              (cont->*KeyGetter)()};
            if(!contRange.inRange(currentFrame)) {
                result.append({currentFrame,
                               qMin(range.fMax, contRange.fMin - 1)});
            }
            currentFrame = contRange.fMax + 1;
        }

        return result;
    }

    bool isEmpty() const { return mList.isEmpty(); }

    void add(const TCont& element) {
        const int contId = idAtOrAfterFrame(((*element).*KeyGetter)());
        if(contId == -1) mList.append(element);
        else mList.insert(contId, element);
    }

    template <class U = T>
    U* atId(const int id) const {
        return static_cast<U*>(&*mList.at(id));
    }

    template <class U = T>
    U* first() const {
        return static_cast<U*>(&*mList.first());
    }

    template <class U = T>
    U* last() const {
        return static_cast<U*>(&*mList.last());
    }

    template <class U = T>
    U* atFrame(const int frame) const {
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return nullptr;
        if(((**notPrev).*KeyGetter)() == frame)
            return static_cast<U*>(&**notPrev);
        return nullptr;
    }

    template <class U = T>
    U* atOrBeforeFrame(const int frame) const {
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return nullptr;
        if(((**notPrev).*KeyGetter)() == frame)
            return static_cast<U*>(&**notPrev);
        return static_cast<U*>(&**(notPrev - 1));
    }

    template <class U = T>
    U* atOrAfterFrame(const int frame) const {
        return static_cast<U*>(&**lower_bound(frame));
    }

    int idAtFrame(const int frame) const {
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return -1;
        if(((**notPrev).*KeyGetter)() == frame)
            return notPrev - begin();
        return -1;
    }

    int idAtOrBeforeFrame(const int frame) const {
        const auto notPrev = lower_bound(frame);
        if(notPrev != mList.end() &&
           (notPrev->get()->*KeyGetter)() == frame)
            return notPrev - begin();
        return notPrev - begin() - 1;
    }

    int idAtOrAfterFrame(const int frame) const {
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return -1;
        return notPrev - begin();
    }

    IdRange rangeToIdRange(const FrameRange &range) {
        return {idAtOrAfterFrame(range.fMin),
                idAtOrBeforeFrame(range.fMax)};
    }
private:
    QList<TCont> mList;
};
#endif // ATOMICSET_H
