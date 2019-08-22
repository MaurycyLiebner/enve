#ifndef QCUBICSEGMENT1D_H
#define QCUBICSEGMENT1D_H
#include <QPointF>
#include <QPainterPath>
#include "../simplemath.h"
#include "../basicreadwrite.h"
typedef std::pair<qreal, qreal> qrealPair;

struct qCubicSegment1D {
    typedef std::pair<qCubicSegment1D, qCubicSegment1D> Pair;
    qCubicSegment1D() {}

    qCubicSegment1D(const qreal p) {
        mP0 = p; mC1 = p; mC2 = p; mP1 = p;
    }

    qCubicSegment1D(const qreal p0, const qreal c1,
                    const qreal c2, const qreal p1){
        mP0 = p0; mC1 = c1; mC2 = c2; mP1 = p1;
    }

    static qCubicSegment1D sMakeLinearToT(const qreal p0, const qreal p1) {
        const qreal dist = p1 - p0;
        return qCubicSegment1D(p0, p0 + dist/3, p1 - dist/3, p1);
    }

    qreal valAtT(const qreal t) const;

    qreal length();
    qreal tAtLength(const qreal len);
    qreal lengthAtT(qreal t);
    qreal lengthFracAtT(qreal t);

    Pair dividedAtT(qreal t);
    qCubicSegment1D tFragment(qreal minT, qreal maxT);

    qreal p0() const;
    qreal c1() const;
    qreal c2() const;
    qreal p1() const;

    void setP0(const qreal p0);
    void setC1(const qreal c1);
    void setC2(const qreal c2);
    void setP1(const qreal p1);

    qreal minDistanceTo(const qreal p,
                        qreal * const pBestT = nullptr,
                        qreal * const pBestPos = nullptr);
    qreal minDistanceTo(const qreal p,
                        const qreal minT,
                        const qreal maxT,
                        qreal * const pBestT = nullptr,
                        qreal * const pBestPos = nullptr);

    bool operator==(const qCubicSegment1D& seg) const {
        return isZero6Dec(seg.p0() - p0()) &&
               isZero6Dec(seg.c1() - c1()) &&
               isZero6Dec(seg.c2() - c2()) &&
               isZero6Dec(seg.p1() - p1());
    }

    bool operator!=(const qCubicSegment1D& seg) const {
        return !this->operator==(seg);
    }

    qCubicSegment1D operator+(const qCubicSegment1D& b) const {
        return qCubicSegment1D(p0() + b.p0(),
                               c1() + b.c1(),
                               c2() + b.c2(),
                               p1() + b.p1());
    }

    qCubicSegment1D operator*(const qreal b) const {
        return qCubicSegment1D(p0()*b, c1()*b, c2()*b, p1()*b);
    }

    qCubicSegment1D& operator*=(const qreal b) {
        setP0(p0()*b);
        setC1(c1()*b);
        setC2(c2()*b);
        setP1(p1()*b);
        return *this;
    }

    qCubicSegment1D operator/(const qreal b) const {
        return qCubicSegment1D(p0()/b, c1()/b, c2()/b, p1()/b);
    }

    qCubicSegment1D& operator/=(const qreal b) {
        setP0(p0()/b);
        setC1(c1()/b);
        setC2(c2()/b);
        setP1(p1()/b);
        return *this;
    }
    qreal maxValue() const;
    qreal minValue() const;

    qreal tWithBiggestValue() const;
    qreal tWithSmallestValue() const;
//    static QList<qrealPair> sIntersectionTs(_qCubicSegment2D& seg1,
//                                            _qCubicSegment2D& seg2);
private:
    qreal tAtLength(const qreal length, const qreal maxLenErr,
                    const qreal minT, const qreal maxT);
    void solveDerivativeZero(qreal &t1, qreal &t2, qreal &t3) const;

    void updateLength();
    bool mLengthUpToDate = false;
    qreal fLength;

    qreal mP0;
    qreal mC1;
    qreal mC2;
    qreal mP1;
};

eWriteStream& operator<<(eWriteStream& dst, const qCubicSegment1D& seg);
eReadStream& operator>>(eReadStream& src, qCubicSegment1D& seg);

#endif // QCUBICSEGMENT1D_H
