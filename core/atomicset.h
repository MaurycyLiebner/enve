#ifndef ATOMICSET_H
#define ATOMICSET_H

#include <QList>
#include "framerange.h"

template <class T>
struct RawVal {
    T fVal;

    T& operator*() const { return fVal; }
};

template <class T, const int& (T::*FrameGetter)() const,
          class TCont = RawVal<T>>
class AtomicSet {
    struct ContFrameLess {
        bool operator()(const TCont& element, const int frame) {
            return ((*element).*FrameGetter)() < frame;
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
            currFrame = (cont->*FrameGetter)() + 1;
        }
    }

    int firstEmptyFrameAtOrAfterFrame(const int frame) const {
        int currFrame = frame;
        T *cont = nullptr;
        while(true) {
            cont = atFrame(currFrame);
            if(!cont) return currFrame;
            currFrame = (cont->*FrameGetter)() + 1;
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
            const auto contRange = FrameRange{(cont->*FrameGetter)(),
                                              (cont->*FrameGetter)()};
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
        const int contId = idAtOrAfterFrame(((*element).*FrameGetter)());
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
        if(isEmpty()) return nullptr;
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return nullptr;
        if(((**notPrev).*FrameGetter)() == frame)
            return static_cast<U*>(&**notPrev);
        return nullptr;
    }

    template <class U = T>
    U* atOrBeforeFrame(const int frame) const {
        if(isEmpty()) return nullptr;
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return static_cast<U*>(&**(end() - 1));
        if(((**notPrev).*FrameGetter)() == frame)
            return static_cast<U*>(&**notPrev);
        if(notPrev == begin()) return nullptr;
        return static_cast<U*>(&**(notPrev - 1));
    }

    template <class U = T>
    U* atOrAfterFrame(const int frame) const {
        if(isEmpty()) return nullptr;
        return static_cast<U*>(&**lower_bound(frame));
    }

    template <class U = T>
    U* beforeFrame(const int frame) const {
        if(isEmpty()) return nullptr;
        const auto notPrev = lower_bound(frame);
        if(notPrev == begin()) return nullptr;
        return static_cast<U*>(&**(notPrev - 1));
    }

    template <class U = T>
    U* afterFrame(const int frame) const {
        if(isEmpty()) return nullptr;
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return nullptr;
        if(((**notPrev).*FrameGetter)() == frame) {
            if(notPrev + 1 == end()) return nullptr;
            return static_cast<U*>(&**(notPrev + 1));
        }
        return static_cast<U*>(&**(notPrev));
    }

    int idAtFrame(const int frame) const {
        if(isEmpty()) return -1;
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return -1;
        if(((**notPrev).*FrameGetter)() == frame)
            return notPrev - begin();
        return -1;
    }

    int idAtOrBeforeFrame(const int frame) const {
        if(isEmpty()) return -1;
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return count() - 1;
        if(((**notPrev).*FrameGetter)() == frame)
            return notPrev - begin();
        return notPrev - begin() - 1;
    }

    int idAtOrAfterFrame(const int frame) const {
        if(isEmpty()) return -1;
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return -1;
        return notPrev - begin();
    }

    int idBeforeFrame(const int frame) const {
        if(isEmpty()) return -1;
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return count() - 1;
        if(((**notPrev).*FrameGetter)() == frame) {
            return notPrev - 1 - begin();
        }
        if(notPrev - begin() < 2) return -1;
        return notPrev - 2 - begin();
    }

    int idAfterFrame(const int frame) const {
        if(isEmpty()) return -1;
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return -1;
        if(((**notPrev).*FrameGetter)() == frame) {
            if(notPrev + 1 == end()) return -1;
            return notPrev + 1 - begin();
        }
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
