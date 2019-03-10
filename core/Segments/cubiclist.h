#ifndef CUBICLIST_H
#define CUBICLIST_H
#include "Segments/qcubicsegment2d.h"
#include <QList>
#include "simplemath.h"

struct CubicList {
    CubicList();
    CubicList(const QList<qCubicSegment2D>& segs);
    CubicList(const CubicList& src);

    CubicList getFragment(const double &minLenFrac,
                          const double &maxLenFrac);

    CubicList getFragmentUnbound(const double &minLenFrac,
                                 const double &maxLenFrac);

    static QList<CubicList> sMakeFromSkPath(const SkPath& src);

    SkPath toSkPath() {
        if(isEmpty()) return SkPath();
        SkPath path;
        bool first = true;
        QPointF lastPos;
        QPointF firstPos;
        for(const auto& cubic : mSegments) {
            if(first) {
                first = false;
                firstPos = cubic.p0();
                path.moveTo(qPointToSk(firstPos));
            } else if(pointToLen(cubic.p0() - lastPos) > 0.1)  {
                firstPos = cubic.p0();
                path.moveTo(qPointToSk(firstPos));
            }
            path.cubicTo(qPointToSk(cubic.c1()),
                         qPointToSk(cubic.c2()),
                         qPointToSk(cubic.p1()));
            lastPos = cubic.p1();
        }
        return path;
    }

    qreal getTotalLength();

    bool isEmpty() const;
    bool isClosed() const {
        return mClosed;
    }

    qreal minDistanceTo(const QPointF &p,
                        qreal * const pBestT = nullptr,
                        QPointF * const pBestPos = nullptr);

    void opSmoothOut(const qreal &smoothness);
    void subdivide(const int &sub = 1);

    void smoothOutNode(const int& nodeId);

    void finishedAdding() {
        updateClosed();
    }

    bool isClockWise() const {
        qreal sum = 0;
        for(const auto& seg : mSegments) {
            sum += (seg.p1().x() - seg.p0().x())*(seg.p1().y() + seg.p0().y());
        }
        return sum > 0;
    }

    QList<SkPath> solidified(const qreal& width) const {
        QList<SkPath> result;
        SkStroke strokerSk;
        strokerSk.setJoin(SkPaint::kRound_Join);
        strokerSk.setCap(SkPaint::kRound_Cap);
        strokerSk.setWidth(static_cast<SkScalar>(width));
        for(int i = 0; i < mSegments.count(); i++) {
            auto& cubic = mSegments[i];
            SkPath outline;
            strokerSk.strokePath(cubic.toSkPath(), &outline);
            result << outline;
        }
        return result;
    }

    const QList<qCubicSegment2D>& getSegments() const {
        return mSegments;
    }

    typedef QList<qCubicSegment2D>::const_iterator const_iterator;
    const_iterator cbegin() const { return mSegments.cbegin(); }
    const_iterator cend() const { return mSegments.cend(); }

    const_iterator begin() const { return mSegments.begin(); }
    const_iterator end() const { return mSegments.end(); }
private:
    void updateTotalLength();
    void updateClosed();

    bool mClosedUpToDate = true;
    bool mClosed = false;
    bool mTotalLengthUpToDate = false;
    qreal mTotalLength = 0;
    QList<qCubicSegment2D> mSegments;
};

#endif // CUBICLIST_H
