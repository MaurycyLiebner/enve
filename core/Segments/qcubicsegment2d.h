#ifndef QCUBICSEGMENT2D_H
#define QCUBICSEGMENT2D_H
#include <skia/skiaincludes.h>
#include <QPointF>
#include <QPainterPath>
#include "simplemath.h"
typedef std::pair<qreal, qreal> qrealPair;
struct qCubicSegment1D;
#define qCubicVals2D \
    const qreal& p0x = p0().x(); \
    const qreal& p0y = p0().y(); \
    const qreal& c1x = p1().x(); \
    const qreal& c1y = c1().y(); \
    const qreal& c2x = c2().x(); \
    const qreal& c2y = c2().y(); \
    const qreal& p1x = p1().x(); \
    const qreal& p1y = p1().y();

#define qCubic1DToNamedVals(seg, add) \
    const qreal& p##add##0 = seg.p0(); \
    const qreal& c##add##1 = seg.c1(); \
    const qreal& c##add##2 = seg.c2(); \
    const qreal& p##add##1 = seg.p1();

struct qCubicSegment2D {
    struct PosAndT {
        qreal fT;
        QPointF fPos;
    };

    typedef std::pair<qCubicSegment2D, qCubicSegment2D> Pair;
    qCubicSegment2D(const QPointF& p0, const QPointF& c1,
                    const QPointF& c2, const QPointF& p1) {
        mP0 = p0; mC1 = c1; mC2 = c2; mP1 = p1;
    }

    qCubicSegment2D(const QPointF& p) {
        mP0 = p; mC1 = p; mC2 = p; mP1 = p;
    }

    qCubicSegment2D() : qCubicSegment2D(QPointF(0, 0)) {}

    qCubicSegment2D(const qCubicSegment1D& xSeg,
                    const qCubicSegment1D& ySeg);

    static qCubicSegment2D fromConic(const QPointF& p0, const QPointF& c,
                                     const QPointF& p1, const qreal& weight) {
        qreal u = 4*weight/(3*(1 + weight));
        return qCubicSegment2D(p0, p0*(1 - u) + c*u, p1*(1 - u) + c*u, p1);
    }

    static qCubicSegment2D fromQuad(const QPointF& p0, const QPointF& c,
                                    const QPointF& p1) {
        return qCubicSegment2D(p0, p0 + 2*(c - p0)/3, p1 + 2*(c - p1)/3, p1);
    }

    SkPath toSkPath() const {
        SkPath path;
        path.moveTo(toSkPoint(mP0));
        path.cubicTo(toSkPoint(mC1), toSkPoint(mC2), toSkPoint(mP1));
        return path;
    }

    qCubicSegment1D xSeg() const;
    qCubicSegment1D ySeg() const;

    QPointF posAtT(const qreal& t) const;
    QPointF posAtLength(const qreal &len);
    qreal tAtPos(const QPointF& pos);

    qreal length();
    qreal tAtLength(const qreal& len);
    qreal lengthAtT(qreal t);
    qreal lengthFracAtT(qreal t);

    Pair dividedAtT(qreal t) const;

    const QPointF &p0() const;
    const QPointF &c1() const;
    const QPointF &c2() const;
    const QPointF &p1() const;

    void setP0(const QPointF& p0);
    void setC1(const QPointF& c1);
    void setC2(const QPointF& c2);
    void setP1(const QPointF& p1);

    qreal tValueForPointClosestTo(const QPointF& p);
    PosAndT closestPosAndT(const QPointF& p);
    qreal minDistanceTo(const QPointF &p,
                        qreal * const pBestT = nullptr,
                        QPointF * const pBestPos = nullptr);
    qreal minDistanceTo(const QPointF &p,
                        const qreal &minT,
                        const qreal &maxT,
                        qreal * const pBestT = nullptr,
                        QPointF * const pBestPos = nullptr);

    qCubicSegment2D rotated(const qreal& deg) const {
        qCubicSegment2D result(*this);
        result.rotate(deg);
        return result;
    }

    void rotate(const qreal& deg) {
        if(isZero6Dec(deg)) return;
        mP0 = gRotPt(p0(), deg);
        mC1 = gRotPt(c1(), deg);
        mC2 = gRotPt(c2(), deg);
        mP1 = gRotPt(p1(), deg);
        mLengthUpToDate = false;
    }

    qCubicSegment2D randomDisplaced(const qreal &displ) {
        qCubicSegment2D result(*this);
        result.randomDisplace(displ);
        return result;
    }

    void randomDisplace(const qreal &displ) {
        setP0(gQPointFDisplace(p0(), displ));
        setC1(gQPointFDisplace(c1(), displ));
        setC2(gQPointFDisplace(c2(), displ));
        setP1(gQPointFDisplace(p1(), displ));
        mLengthUpToDate = false;
    }

    SkPath solidified(const qreal& width) const {
        SkStroke strokerSk;
        strokerSk.setJoin(SkPaint::kRound_Join);
        strokerSk.setCap(SkPaint::kRound_Cap);
        strokerSk.setWidth(static_cast<SkScalar>(width));
        SkPath outline;
        strokerSk.strokePath(toSkPath(), &outline);
        return outline;
    }

    // -90 is y direction 0 is x direction
    qreal tFurthestInDirection(const qreal& deg) const;
//    static QList<qrealPair> sIntersectionTs(_qCubicSegment2D& seg1,
//                                            _qCubicSegment2D& seg2);

    qCubicSegment2D tFragment(qreal minT, qreal maxT) const;
    qCubicSegment2D lenFragment(const qreal& minLen, const qreal& maxLen);
    qCubicSegment2D lenFracFragment(const qreal& minLenFrac,
                                    const qreal& maxLenFrac);

    bool isLine() const {
        const qreal arr1 = mP0.x()*(mC1.y() - mC2.y()) +
                           mC1.x()*(mC2.y() - mP0.y()) +
                           mC2.x()*(mP0.y() - mC1.y());
        if(!isZero2Dec(arr1)) return false;
        const qreal arr2 = mP1.x()*(mC1.y() - mC2.y()) +
                           mC1.x()*(mC2.y() - mP1.y()) +
                           mC2.x()*(mP1.y() - mC1.y());
        if(!isZero2Dec(arr2)) return false;
        return true;
    }

    bool isNull() const {
        if(!isZero2Dec(pointToLen(mP0 - mP1))) return false;
        if(!isZero2Dec(pointToLen(mP0 - mC1))) return false;
        if(!isZero2Dec(pointToLen(mP0 - mC2))) return false;
        return true;
    }
private:
    qreal tAtLength(const qreal& length, const qreal& maxLenErr,
                    const qreal& minT, const qreal& maxT);

    void updateLength();
    bool mLengthUpToDate = false;
    qreal fLength;

    QPointF mP0;
    QPointF mC1;
    QPointF mC2;
    QPointF mP1;
};

#endif // QCUBICSEGMENT2D_H
