#ifndef HDDCACHABLECACHEHANDLER_H
#define HDDCACHABLECACHEHANDLER_H
#include "minimalcachehandler.h"
#include "hddcachablerangecont.h"
#include "rangemap.h"

class HDDCachableCacheHandler {
public:
    typedef HDDCachableRangeCont Cont;

    void blockConts(const FrameRange &range, const bool blocked);

    void drawCacheOnTimeline(QPainter * const p,
                             const QRectF &drawRect,
                             const int startFrame,
                             const int endFrame,
                             const qreal unit = 1) const;

    int firstEmptyFrameAtOrAfter(const int frame) const {
        return mConts.firstEmptyRangeLowerBound(frame).fMin;
    }

    void add(const stdsptr<Cont>& cont) {
        const auto ret = mConts.insert({cont->getRange(), cont});
        if(!ret.second) RuntimeThrow("Range already occupied by a different element");
    }

    void clear() {
        mConts.clear();
    }

    void remove(const stdsptr<Cont>& cont) {
        mConts.erase(cont->getRange());
    }

    void remove(const iValueRange& range) {
        mConts.erase(range);
    }

    template <class T = Cont>
    T * atFrame(const int relFrame) const {
        const auto it = mConts.atFrame(relFrame);
        if(it == mConts.end()) return nullptr;
        return static_cast<T*>(it->second.get());
    }

    template <class T = Cont>
    T * atOrBeforeFrame(const int relFrame) const {
        const auto it = mConts.atOrBeforeFrame(relFrame);
        if(it == mConts.end()) return nullptr;
        return static_cast<T*>(it->second.get());
    }
private:
    RangeMap<stdsptr<Cont>> mConts;
};

#endif // HDDCACHABLECACHEHANDLER_H
