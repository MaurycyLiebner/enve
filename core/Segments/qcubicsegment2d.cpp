#include "qcubicsegment2d.h"
#include "qcubicsegment1d.h"
#include "simplemath.h"

qCubicSegment2D::qCubicSegment2D(const qCubicSegment1D &xSeg,
                                 const qCubicSegment1D &ySeg) {
    mP0 = QPointF(xSeg.p0(), ySeg.p0());
    mC1 = QPointF(xSeg.c1(), ySeg.c1());
    mC2 = QPointF(xSeg.c2(), ySeg.c2());
    mP1 = QPointF(xSeg.c1(), ySeg.c1());
}

qCubicSegment1D qCubicSegment2D::xSeg() const {
    return qCubicSegment1D(mP0.x(), mC1.x(), mC2.x(), mP1.x());
}

qCubicSegment1D qCubicSegment2D::ySeg() const {
    return qCubicSegment1D(mP0.y(), mC1.y(), mC2.y(), mP1.y());
}

QPointF qCubicSegment2D::posAtT(const qreal &t) const {
    return qPow(1 - t, 3)*p0() +
            3*qPow(1 - t, 2)*t*c1() +
            3*(1 - t)*t*t*c2() +
            t*t*t*p1();
}

qreal qCubicSegment2D::length() {
    if(!mLengthUpToDate) updateLength();
    return fLength;
}

qreal qCubicSegment2D::tAtLength(const qreal &len) {
    if(isZero6Dec(len) || len < 0) return 0;
    qreal totLen = length();
    if(isZero6Dec(len - totLen) || len > totLen) return 1;
    return tAtLength(len, 0.01, 0, 1);
}

qreal qCubicSegment2D::lengthAtT(qreal t) {
    t = CLAMP(t, 0, 1);
    if(isZero6Dec(qMax(0., t))) return 0;
    if(isZero6Dec(qMin(1., t) - 1)) return length();
    auto divSeg = dividedAtT(t);
    return divSeg.first.length();
}

qreal qCubicSegment2D::lengthFracAtT(qreal t) {
    t = CLAMP(t, 0, 1);
    if(isZero6Dec(t)) return 0;
    if(isZero6Dec(t - 1)) return length();
    qreal totLen = length();
    if(isZero6Dec(totLen)) return 1;
    return lengthAtT(t)/totLen;
}

qCubicSegment2D::Pair qCubicSegment2D::dividedAtT(qreal t) const {
    t = CLAMP(t, 0, 1);
    if(isZero6Dec(t)) return { qCubicSegment2D(mP0), *this };
    if(isZero6Dec(t - 1)) return { *this, qCubicSegment2D(mP1) };
    qreal oneMinusT = 1 - t;
    QPointF P0_1 = p0()*oneMinusT + c1()*t;
    QPointF P1_2 = c1()*oneMinusT + c2()*t;
    QPointF P2_3 = c2()*oneMinusT + p1()*t;

    QPointF P01_12 = P0_1*oneMinusT + P1_2*t;
    QPointF P12_23 = P1_2*oneMinusT + P2_3*t;

    QPointF P0112_1223 = P01_12*oneMinusT + P12_23*t;

    qCubicSegment2D seg1(p0(), P0_1, P01_12, P0112_1223);
    qCubicSegment2D seg2(P0112_1223, P12_23, P2_3, p1());

    return {seg1, seg2};
}

const QPointF &qCubicSegment2D::p0() const { return mP0; }

const QPointF &qCubicSegment2D::c1() const { return mC1; }

const QPointF &qCubicSegment2D::c2() const { return mC2; }

const QPointF &qCubicSegment2D::p1() const { return mP1; }

void qCubicSegment2D::setP0(const QPointF &p0) {
    mP0 = p0;
    mLengthUpToDate = false;
}

void qCubicSegment2D::setC1(const QPointF &c1) {
    mC1 = c1;
    mLengthUpToDate = false;
}

void qCubicSegment2D::setC2(const QPointF &c2) {
    mC2 = c2;
    mLengthUpToDate = false;
}

void qCubicSegment2D::setP1(const QPointF &p1) {
    mP1 = p1;
    mLengthUpToDate = false;
}

qreal qCubicSegment2D::minDistanceTo(const QPointF &p,
                                         qreal * const pBestT,
                                         QPointF * const pBestPos) {
    return minDistanceTo(p, 0, 1, pBestT, pBestPos);
}

qreal qCubicSegment2D::minDistanceTo(const QPointF &p,
                                     const qreal &minT,
                                     const qreal &maxT,
                                     qreal * const pBestT,
                                     QPointF * const pBestPos) {
    qreal maxLen = lengthAtT(maxT);
    qreal bestT = 0;
    QPointF bestPt = p0();
    qreal minError = DBL_MAX;
    for(qreal len = lengthAtT(minT); len < maxLen;) { // t ∈ [0., 1.]
        qreal t = tAtLength(len);
        QPointF pt = posAtT(t);
        qreal dist = pointToLen(pt - p);
        if(dist < minError) {
            bestT = t;
            bestPt = pt;
            minError = dist;
            if(minError < 1) {
                qCubicVals2D;
                while(dist < minError) {
                    bestT = t;
                    bestPt = pt;
                    minError = dist;
                    qreal tMinusOne = t - 1;
                    qreal pow2TMinusOne = pow2(tMinusOne);
                    qreal pow3TMinusOne = pow3(tMinusOne);

                    qreal v0 = 3*c2x - 3*c2x*t + p1x*t;
                    qreal v1 = 3*c2y - 3*c2y*t + p1y*t;

                    qreal v2 = 3*c1x*pow2TMinusOne + t*v0;
                    qreal v3 = t*(3*c1y*pow2TMinusOne + t*v1);

                    qreal v4 = -1 + 4*t - 3*pow2(t);

                    qreal num = pow2(p.x() + p0x*pow3TMinusOne - t*v2) +
                                    pow2(p.y() + p0y*pow3TMinusOne - v3);
                    qreal den = -6*(p0x*pow2TMinusOne + t*(-2*c2x + 3*c2x*t - p1x*t) +
                                    c1x*v4)*
                                  (-p.x() - p0x*pow3TMinusOne +  t*v2) -
                                 6*(p0y*pow2TMinusOne + t*(-2*c2y + 3*c2y*t - p1y*t) +
                                    c1y*v4)*
                                  (-p.y() - p0y*pow3TMinusOne + v3);
                    if(isZero6Dec(den)) {
//                        if(first && minError > 0.01 && !isZeroOrOne6Dec(t)) {
//                            t = gCubicTimeAtLength(seg, len + dist*0.5);
//                            pt = gCalcCubicBezierVal(seg, t);
//                            dist = pointToLen(pt - p);
//                            continue;
//                        }
                        break;
                    }
                    qreal newT = t - num/den;
                    pt = posAtT(newT);
                    dist = pointToLen(pt - p);
                    t = newT;
                }
            }
            if(minError < 0.01) break;
        }

        len += dist*0.8;
    }
    if(pBestPos) *pBestPos = bestPt;
    if(pBestT) *pBestT = bestT;
    return minError;
}

qreal qCubicSegment2D::tFurthestInDirection(const qreal &deg) const {
    auto rotSeg = rotated(deg);
    auto xS = rotSeg.xSeg();
    qCubic1DToNamedVals(xS, x);
    bool p0Further = px0 >= cx1 && px0 >= cx2;
    bool p3Further = px1 >= cx1 && px1 >= cx2;
    if(p0Further || p3Further) {
        if(px1 > px0) return 1;
        else return 0;
    }
    qreal den = px0 - 3*cx1 + 3*cx2 - px1;
    qreal num0 = px0 - 2*cx1 + cx2;
    qreal numSqrt = sqrt(pow2(cx1) - px0*cx2 - cx1*cx2 +
                         pow2(cx2) + px0*px1 - cx1*px1);
    qreal t1 = (num0 + numSqrt)/den;
    qreal t2 = (num0 - numSqrt)/den;
    if(t1 > 0 || t1 < 1) return t1;
    if(t2 > 0 || t2 < 1) return t2;
    Q_ASSERT(false);
    return 0;
}
#include "exceptions.h"
qCubicSegment2D qCubicSegment2D::tFragment(qreal minT, qreal maxT) const {
    maxT = CLAMP01(maxT);
    minT = CLAMP01(minT);
    if(isZero6Dec(minT - maxT)) return qCubicSegment2D(posAtT(minT));
    if(minT > maxT) {
        RuntimeThrow("Wrong t range. Min value larger than max.");
    }
    if(isZero6Dec(minT - 1)) return qCubicSegment2D(mP1);
    qCubicSegment2D div1 = dividedAtT(minT).second;
    if(isZero6Dec(maxT - 1)) return div1;
    qreal mappedMaxT = (maxT - minT)/(1 - minT);
    return div1.dividedAtT(mappedMaxT).first;
}

qCubicSegment2D qCubicSegment2D::lenFragment(const qreal &minLen,
                                             const qreal &maxLen) {
    return tFragment(tAtLength(minLen), tAtLength(maxLen));
}

qCubicSegment2D qCubicSegment2D::lenFracFragment(
        const qreal& minLenFrac, const qreal& maxLenFrac) {
    return lenFragment(minLenFrac*length(), maxLenFrac*length());
}

//QList<qrealPair> _qCubicSegment2D::sIntersectionTs(
//        _qCubicSegment2D &seg1, _qCubicSegment2D &seg2) {
//    QList<qrealPair> sols;
//    qreal totalLen1 = seg1.getLength();
//    qreal totalLen2 = seg2.getLength();

//    for(qreal len1 = 0; len1 < totalLen1;) { // t ∈ (0., 1.)
//        qreal t1 = seg1.tAtLength(len1);
//        QPointF pt1 = seg1.posAtT(t1);

//        qreal smallestDist = DBL_MAX;
//        for(qreal len2 = 0; len2 < totalLen2;) { // t ∈ (0., 1.)
//            qreal t2 = seg2.tAtLength(len2);
//            QPointF pt2 = seg2.posAtT(t2);
//            qreal dist = pointToLen(pt1 - pt2);
//            if(dist < smallestDist) smallestDist = dist;

//            if(dist < .5) {
//                sols.append({t1, t2});
//                len2 += 1.;
//                smallestDist += 1;
//                break;
//            }

//            len2 += dist*0.5;
//        }

//        len1 += smallestDist*0.5;
//    }

//    return sols;
//}

qreal qCubicSegment2D::tAtLength(const qreal &length,
                                 const qreal &maxLenErr,
                                 const qreal &minT,
                                 const qreal &maxT) {
    qreal guessT = (maxT + minT)*0.5;
    qreal lenAtGuess = lengthAtT(guessT);
    if(abs(lenAtGuess - length) < maxLenErr) return guessT;
    if(lenAtGuess > length) {
        return tAtLength(length, maxLenErr, minT, guessT);
    } else {
        return tAtLength(length, maxLenErr, guessT, maxT);
    }
}

void qCubicSegment2D::updateLength() {
    mLengthUpToDate = true;
    QPainterPath path;
    path.moveTo(p0());
    path.cubicTo(c1(), c2(), p1());
    fLength = path.length();
}
