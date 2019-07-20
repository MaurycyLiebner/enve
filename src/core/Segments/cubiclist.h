#ifndef CUBICLIST_H
#define CUBICLIST_H
#include "Segments/qcubicsegment2d.h"
#include <QList>
#include "simplemath.h"

struct CubicList {
    CubicList();
    CubicList(const QList<qCubicSegment2D>& segs);
    CubicList(const CubicList& src);

    PosAndTan posAndTanAtLength(const qreal atLength) {
        if(atLength < 0) {
            if(mSegments.isEmpty()) return { QPointF(0, 0), QPointF(0, 0) };
            return { mSegments.first().p0(), mSegments.first().tanAtT(0) };
        }
        qreal currLen = 0;
        const int iMax = mSegments.count();
        for(int i = 0; i < iMax; i++) {
            auto& seg = mSegments[i];
            const qreal lastLen = currLen;
            currLen += seg.length();
            if(currLen > atLength)
                return seg.posAndTanAtLength(atLength - lastLen);
        }
        if(mSegments.isEmpty()) return { QPointF(0, 0), QPointF(0, 0) };
        return { mSegments.last().p3(), mSegments.last().tanAtT(1) };
    }

    QPointF posAtLength(const qreal atLength) {
        if(atLength < 0) {
            if(mSegments.isEmpty()) return QPointF(0, 0);
            return mSegments.first().p0();
        }
        qreal currLen = 0;
        const int iMax = mSegments.count();
        for(int i = 0; i < iMax; i++) {
            auto& seg = mSegments[i];
            const qreal lastLen = currLen;
            currLen += seg.length();
            if(currLen > atLength)
                return seg.posAtLength(atLength - lastLen);
        }
        if(mSegments.isEmpty()) return QPointF(0, 0);
        return mSegments.last().p3();
    }

    QPointF tanAtLength(const qreal atLength) {
        if(atLength < 0) {
            if(mSegments.isEmpty()) return QPointF(0, 0);
            return mSegments.first().tanAtT(0);
        }
        qreal currLen = 0;
        const int iMax = mSegments.count();
        for(int i = 0; i < iMax; i++) {
            auto& seg = mSegments[i];
            const qreal lastLen = currLen;
            currLen += seg.length();
            if(currLen > atLength)
                return seg.tanAtLength(atLength - lastLen);
        }
        if(mSegments.isEmpty()) return QPointF(0, 0);
        return mSegments.last().tanAtT(1);
    }

    CubicList getFragment(const qreal minLenFrac,
                          const qreal maxLenFrac);

    CubicList getFragmentUnbound(const qreal minLenFrac,
                                 const qreal maxLenFrac);

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
                path.moveTo(toSkPoint(firstPos));
            } else if(pointToLen(cubic.p0() - lastPos) > 0.1)  {
                firstPos = cubic.p0();
                path.moveTo(toSkPoint(firstPos));
            }
            path.cubicTo(toSkPoint(cubic.c1()),
                         toSkPoint(cubic.c2()),
                         toSkPoint(cubic.p3()));
            lastPos = cubic.p3();
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

    void opSmoothOut(const qreal smoothness);
    void subdivide(const int sub = 1);

    void finishedAdding() {
        updateClosed();
    }

    bool isClockWise() const {
        qreal sum = 0;
        for(const auto& seg : mSegments) {
            sum += (seg.p3().x() - seg.p0().x())*(seg.p3().y() + seg.p0().y());
        }
        return sum > 0;
    }

    const QList<qCubicSegment2D>& getSegments() const {
        return mSegments;
    }

    int lineIntersections(const QLineF& line, QList<QPointF>& pts);

    typedef QList<qCubicSegment2D>::const_iterator const_iterator;
    const_iterator cbegin() const { return mSegments.cbegin(); }
    const_iterator cend() const { return mSegments.cend(); }

    typedef QList<qCubicSegment2D>::iterator iterator;
    iterator begin() { return mSegments.begin(); }
    iterator end() { return mSegments.end(); }
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
