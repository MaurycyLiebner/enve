#ifndef CUBICLIST_H
#define CUBICLIST_H
#include "Segments/cubicsegment.h"
#include <QList>
#include "simplemath.h"

struct CubicList {
    CubicList();
    CubicList(const QList<qCubicSegment2D>& segs);

    CubicList getFragment(const double &minLenFrac,
                          const double &maxLenFrac);

    static QList<CubicList> makeFromSkPath(const SkPath& src);

    qreal getTotalLength();

    bool isEmpty() const;

    qreal minDistanceTo(const QPointF &p,
                        qreal * const pBestT = nullptr,
                        QPointF * const pBestPos = nullptr);

    void opSmoothOut(const qreal &smoothness);
    void subdivide(const int &sub = 1);

    void smoothOutNode(const int& nodeId);
private:
    void updateTotalLength();

    void updateClosed();

    bool mClosedUpToDate = true;
    bool mClosed = false;
    bool mTotalLengthUpToDate = true;
    qreal mTotalLength = 0;
    QList<qCubicSegment2D> mSegments;
};

#endif // CUBICLIST_H
