#ifndef QCUBICSEGMENT1D_H
#define QCUBICSEGMENT1D_H
#include <skia/skiaincludes.h>
#include <QPointF>
#include <QPainterPath>
typedef std::pair<qreal, qreal> qrealPair;

struct qCubicSegment1D {
    typedef std::pair<qCubicSegment1D, qCubicSegment1D> Pair;
    qCubicSegment1D(const qreal& p0, const qreal& c1,
                    const qreal& c2, const qreal& p1){
        mP0 = p0; mC1 = c1; mC2 = c2; mP1 = p1;
    }


    qreal valAtT(const qreal& t) const;

    qreal length();
    qreal tAtLength(const qreal& len);
    qreal lengthAtT(qreal t);
    qreal lengthFracAtT(qreal t);

    Pair dividedAtT(qreal t);

    const qreal &p0() const;
    const qreal &c1() const;
    const qreal &c2() const;
    const qreal &p1() const;

    void setP0(const qreal& p0);
    void setC1(const qreal& c1);
    void setC2(const qreal& c2);
    void setP1(const qreal& p1);

    qreal minDistanceTo(const qreal &p,
                        qreal * const pBestT = nullptr,
                        qreal * const pBestPos = nullptr);
    qreal minDistanceTo(const qreal &p,
                        const qreal &minT,
                        const qreal &maxT,
                        qreal * const pBestT = nullptr,
                        qreal * const pBestPos = nullptr);

//    static QList<qrealPair> sIntersectionTs(_qCubicSegment2D& seg1,
//                                            _qCubicSegment2D& seg2);
private:
    qreal tAtLength(const qreal& length, const qreal& maxLenErr,
                    const qreal& minT, const qreal& maxT);

    void updateLength();
    bool mLengthUpToDate = false;
    qreal fLength;

    qreal mP0;
    qreal mC1;
    qreal mC2;
    qreal mP1;
};

#endif // QCUBICSEGMENT1D_H
