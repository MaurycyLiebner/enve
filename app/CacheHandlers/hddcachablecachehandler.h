#ifndef HDDCACHABLECACHEHANDLER_H
#define HDDCACHABLECACHEHANDLER_H
#include "minimalcachehandler.h"
#include "hddcachablerangecont.h"

template <class T, const FrameRange& (T::*RangeGetter)() const>
class RangeSet {
    struct ContFrameLess {
        bool operator()(const stdsptr<T>& element, const int relFrame) {
            return (element.get()->*RangeGetter)().fMax < relFrame;
        }
    };

    void removeId(const int id) {
        mList.removeAt(id);
    }
public:
    typedef typename QList<stdsptr<T>>::iterator iterator;
    typedef typename QList<stdsptr<T>>::const_iterator const_iterator;

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

    bool remove(const stdsptr<T>& element) {
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

    int countAfterFrame(const int relFrame) const {
        const int firstId = idAtOrAfterFrame(relFrame + 1);
        return mList.count() - firstId;
    }

    int getFirstEmptyFrameAfterFrame(const int frame) const {
        int currFrame = frame + 1;
        T *cont = nullptr;
        while(true) {
            cont = atFrame(currFrame);
            if(!cont) return currFrame;
            currFrame = (cont->*RangeGetter)().fMax + 1;
        }
    }

    int firstEmptyFrameAtOrAfterFrame(const int frame) const {
        int currFrame = frame;
        T *cont = nullptr;
        while(true) {
            cont = atFrame(currFrame);
            if(!cont) return currFrame;
            currFrame = (cont->*RangeGetter)().fMax + 1;
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
            auto contRange = (cont->*RangeGetter)();
            if(!contRange.inRange(currentFrame)) {
                result.append({currentFrame,
                               qMin(range.fMax, contRange.fMin - 1)});
            }
            currentFrame = contRange.fMax + 1;
        }

        return result;
    }

    bool isEmpty() const { return mList.isEmpty(); }

    void add(const stdsptr<T>& element) {
        const int contId = idAtOrAfterFrame((element.get()->*RangeGetter)().fMax);
        if(contId == -1) mList.append(element);
        else mList.insert(contId, element);
    }

    template <class U = T>
    U* atId(const int id) const {
        return static_cast<U*>(mList.at(id).get());
    }

    template <class U = T>
    U* first() const {
        return static_cast<U*>(mList.first().get());
    }

    template <class U = T>
    U* last() const {
        return static_cast<U*>(mList.last().get());
    }

    template <class U = T>
    U* atFrame(const int relFrame) const {
        const auto notPrev = lower_bound(relFrame);
        if(notPrev == end()) return nullptr;
        if((notPrev->get()->*RangeGetter)().inRange(relFrame))
            return static_cast<U*>(notPrev->get());
        return nullptr;
    }

    template <class U = T>
    U* atOrBeforeFrame(const int frame) const {
        const auto notPrev = lower_bound(frame);
        if(notPrev == end()) return nullptr;
        if((notPrev->get()->*RangeGetter)().inRange(frame))
            return static_cast<U*>(notPrev->get());
        return static_cast<U*>((notPrev - 1)->get());
    }

    template <class U = T>
    U* atOrAfterFrame(const int frame) const {
        return static_cast<U*>(lower_bound(frame)->get());
    }

    int idAtFrame(const int relFrame) const {
        const auto notPrevious = lower_bound(relFrame);
        if(notPrevious == mList.end()) return -1;
        if((*notPrevious)->*RangeGetter().inRange(relFrame))
            return notPrevious - mList.begin();
        return -1;
    }

    int idAtOrBeforeFrame(const int frame) const {
        const auto notPrev = lower_bound(frame);
        if(notPrev != mList.end() &&
           (notPrev->get()->*RangeGetter)().inRange(frame))
            return notPrev - mList.begin();
        return notPrev - mList.begin() - 1;
    }

    int idAtOrAfterFrame(const int frame) const {
        const auto notPrev = lower_bound(frame);
        if(notPrev == mList.end()) return -1;
        return notPrev - mList.begin();
    }

    IdRange rangeToIdRange(const FrameRange &range) {
        return {idAtOrAfterFrame(range.fMin),
                idAtOrBeforeFrame(range.fMax)};
    }
private:
    QList<stdsptr<T>> mList;
};

class HDDCachableCacheHandler :
        public RangeSet<HDDCachableRangeCont,
                        &HDDCachableRangeCont::getRange> {
public:
    typedef HDDCachableRangeCont Cont;
    typedef QList<stdsptr<Cont>>::const_iterator ContCIter;
    typedef QList<stdsptr<Cont>>::iterator ContIter;

    int getFirstEmptyOrCachedFrameAfterFrame(
            const int frame, Cont **contP = nullptr) const {
        int currFrame = frame + 1;
        Cont *cont = nullptr;
        while(true) {
            cont = atFrame(currFrame);
            if(!cont) break;
            if(!cont->storesDataInMemory()) break;
            currFrame = cont->getRangeMax() + 1;
        }
        if(contP) *contP = cont;
        return currFrame;
    }

    void blockConts(const FrameRange &range, const bool blocked);

    void cacheDataBeforeRelFrame(const int relFrame);
    void cacheDataAfterRelFrame(const int relFrame);

    void cacheFirstContainer();
    void cacheLastContainer();

    void drawCacheOnTimeline(QPainter * const p,
                             const QRectF &drawRect,
                             const int startFrame,
                             const int endFrame,
                             const qreal unit = 1) const;
};

#endif // HDDCACHABLECACHEHANDLER_H
