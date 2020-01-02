// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef QCUBICSEGMENT2D_H
#define QCUBICSEGMENT2D_H
#include "../skia/skqtconversions.h"
#include <QPointF>
#include <QPainterPath>
#include "../simplemath.h"
typedef std::pair<qreal, qreal> qrealPair;
struct qCubicSegment1D;

struct PosAndTan {
    QPointF fPos;
    QPointF fTan;
};

struct PosAndT {
    qreal fT;
    QPointF fPos;
};

struct qCubicSegment2D {
    typedef std::pair<qCubicSegment2D, qCubicSegment2D> Pair;
    qCubicSegment2D(const QPointF& p0, const QPointF& c1,
                    const QPointF& c2, const QPointF& p1) {
        mP0 = p0; mC1 = c1; mC2 = c2; mP3 = p1;
    }

    qCubicSegment2D(const QPointF& p) {
        mP0 = p; mC1 = p; mC2 = p; mP3 = p;
    }

    qCubicSegment2D() : qCubicSegment2D(QPointF(0, 0)) {}

    qCubicSegment2D(const qCubicSegment1D& xSeg,
                    const qCubicSegment1D& ySeg);

    static qCubicSegment2D sFromLine(const QLineF& line) {
        return sFromLine(line.p1(), line.p2());
    }

    static qCubicSegment2D sFromLine(const QPointF& p0, const QPointF& p1) {
        return qCubicSegment2D(p0, p0*(2./3) + p1*(1./3),
                               p0*(1./3) + p1*(2./3), p1);
    }

    static qCubicSegment2D sFromConic(const QPointF& p0, const QPointF& c,
                                      const QPointF& p1, const qreal weight) {
        qreal u = 4*weight/(3*(1 + weight));
        return qCubicSegment2D(p0, p0*(1 - u) + c*u, p1*(1 - u) + c*u, p1);
    }

    static qCubicSegment2D sFromQuad(const QPointF& p0, const QPointF& c,
                                     const QPointF& p1) {
        return qCubicSegment2D(p0, p0 + 2*(c - p0)/3, p1 + 2*(c - p1)/3, p1);
    }

    SkPath toSkPath() const;

    QRectF ptsBoundingRect() const {
        return qRectF4Points(mP0, mC1, mC2, mP3);
    }

    qCubicSegment1D xSeg() const;
    qCubicSegment1D ySeg() const;

    QPointF tanAtLength(const qreal len);
    QPointF tanAtT(const qreal t) const;

    PosAndTan posAndTanAtLength(const qreal len);
    PosAndTan posAndTanAtT(const qreal t) const;

    QPointF posAtT(const qreal t) const;
    QPointF posAtLength(const qreal len);
    qreal tAtPos(const QPointF& pos);

    qreal length();
    qreal tAtLength(const qreal len);
    qreal lengthAtT(qreal t);
    qreal lengthFracAtT(qreal t);

    Pair dividedAtT(qreal t) const;

    const QPointF &p0() const;
    const QPointF &c1() const;
    const QPointF &c2() const;
    const QPointF &p3() const;

    void setP0(const QPointF& p0);
    void setC1(const QPointF& c1);
    void setC2(const QPointF& c2);
    void setP3(const QPointF& p3);

    qreal tValueForPointClosestTo(const QPointF& p);
    PosAndT closestPosAndT(const QPointF& p);
    qreal minDistanceTo(const QPointF &p,
                        qreal * const pBestT = nullptr,
                        QPointF * const pBestPos = nullptr);
    qreal minDistanceTo(const QPointF &p,
                        const qreal minT,
                        const qreal maxT,
                        qreal * const pBestT = nullptr,
                        QPointF * const pBestPos = nullptr);

    qCubicSegment2D rotated(const qreal deg) const {
        qCubicSegment2D result(*this);
        result.rotate(deg);
        return result;
    }

    void rotate(const qreal deg) {
        if(isZero6Dec(deg)) return;
        mP0 = gRotPt(p0(), deg);
        mC1 = gRotPt(c1(), deg);
        mC2 = gRotPt(c2(), deg);
        mP3 = gRotPt(p3(), deg);
        mLengthUpToDate = false;
    }

    void makePassThroughRel(const QPointF &relPos, const qreal t) {
        const qreal oneMinusT = 1 - t;
        QPointF dPos = relPos - posAtT(t);
        while(pointToLen(dPos) > 1) {
            setC1(c1() + oneMinusT*dPos);
            setC2(c2() + t*dPos);
            dPos = relPos - posAtT(t);
        }
    }

    qCubicSegment2D randomDisplaced(const qreal displ) {
        qCubicSegment2D result(*this);
        result.randomDisplace(displ);
        return result;
    }

    void randomDisplace(const qreal displ) {
        setP0(gQPointFDisplace(p0(), displ));
        setC1(gQPointFDisplace(c1(), displ));
        setC2(gQPointFDisplace(c2(), displ));
        setP3(gQPointFDisplace(p3(), displ));
        mLengthUpToDate = false;
    }

    //! @brief -90 is y direction 0 is x direction
    qreal tFurthestInDirection(const qreal deg) const;

    qCubicSegment2D tFragment(qreal minT, qreal maxT) const;
    qCubicSegment2D lenFragment(const qreal minLen, const qreal maxLen);
    qCubicSegment2D lenFracFragment(const qreal minLenFrac,
                                    const qreal maxLenFrac);

    bool isLine() const {
        const qreal arr1 = mP0.x()*(mC1.y() - mC2.y()) +
                           mC1.x()*(mC2.y() - mP0.y()) +
                           mC2.x()*(mP0.y() - mC1.y());
        if(!isZero2Dec(arr1)) return false;
        const qreal arr2 = mP3.x()*(mC1.y() - mC2.y()) +
                           mC1.x()*(mC2.y() - mP3.y()) +
                           mC2.x()*(mP3.y() - mC1.y());
        if(!isZero2Dec(arr2)) return false;
        return true;
    }

    bool isNull() const {
        if(!isZero2Dec(pointToLen(mP0 - mP3))) return false;
        if(!isZero2Dec(pointToLen(mP0 - mC1))) return false;
        if(!isZero2Dec(pointToLen(mP0 - mC2))) return false;
        return true;
    }
private:
    qreal tAtLength(const qreal length, const qreal maxLenErr,
                    const qreal minT, const qreal maxT);

    void updateLength();
    bool mLengthUpToDate = false;
    qreal fLength;

    QPointF mP0;
    QPointF mC1;
    QPointF mC2;
    QPointF mP3;
};

#endif // QCUBICSEGMENT2D_H
