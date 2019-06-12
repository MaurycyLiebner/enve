#ifndef HDDCACHABLECACHEHANDLER_H
#define HDDCACHABLECACHEHANDLER_H
#include "minimalcachehandler.h"
#include "hddcachablerangecont.h"
#include "rangeset.h"

class HDDCachableCacheHandler :
        public RangeSet<HDDCachableRangeCont,
                        &HDDCachableRangeCont::getRange,
                        stdsptr<HDDCachableRangeCont>> {
public:
    typedef HDDCachableRangeCont Cont;
    typedef QList<stdsptr<Cont>>::const_iterator ContCIter;
    typedef QList<stdsptr<Cont>>::iterator ContIter;

    void blockConts(const FrameRange &range, const bool blocked);

    void drawCacheOnTimeline(QPainter * const p,
                             const QRectF &drawRect,
                             const int startFrame,
                             const int endFrame,
                             const qreal unit = 1) const;
};

#endif // HDDCACHABLECACHEHANDLER_H
