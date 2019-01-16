#ifndef CUBICSEGMENT_H
#define CUBICSEGMENT_H
#include <skia/skiaincludes.h>
#include <QPointF>
#include <QPainterPath>
typedef std::pair<qreal, qreal> qrealPair;

#define qCubicVals \
    const qreal& p0x = p0().x(); \
    const qreal& p0y = p0().y(); \
    const qreal& p1x = p1().x(); \
    const qreal& p1y = p1().y(); \
    const qreal& p2x = p2().x(); \
    const qreal& p2y = p2().y(); \
    const qreal& p3x = p3().x(); \
    const qreal& p3y = p3().y();

struct qCubicSegment2D {
    typedef std::pair<qCubicSegment2D, qCubicSegment2D> Pair;

    qCubicSegment2D(const QPointF& p0, const QPointF& p1,
                    const QPointF& p2, const QPointF& p3);

    QPointF posAtT(const qreal& t) const;

    qreal getLength();

    qreal tAtLength(const qreal& length);

    qreal lengthAtT(qreal t);

    qreal lengthFracAtT(qreal t);

    Pair dividedAtT(qreal t);

    const QPointF &p0() const;
    const QPointF &p1() const;
    const QPointF &p2() const;
    const QPointF &p3() const;

    void setP0(const QPointF& p0);
    void setP1(const QPointF& p1);
    void setP2(const QPointF& p2);
    void setP3(const QPointF& p3);

    qreal minDistanceTo(const QPointF &p,
                        qreal * const pBestT,
                        QPointF * const pBestPos);
    qreal minDistanceTo(const QPointF &p,
                        const qreal &minT,
                        const qreal &maxT,
                        qreal * const pBestT,
                        QPointF * const pBestPos);

//    static QList<qrealPair> sIntersectionTs(qCubicSegment2D& seg1,
//                                            qCubicSegment2D& seg2);
private:
    qreal tAtLength(const qreal& length, const qreal& maxLenErr,
                    const qreal& minT, const qreal& maxT);

    void updateLength();
    bool mLengthUpToDate = false;
    qreal fLength;

    QPointF mP0;
    QPointF mP1;
    QPointF mP2;
    QPointF mP3;
};

#endif // CUBICSEGMENT_H
