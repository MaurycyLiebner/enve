#include "cubicsegment.h"
#include "simplemath.h"

qCubicSegment2D::qCubicSegment2D(const QPointF &p0, const QPointF &p1,
                                 const QPointF &p2, const QPointF &p3) {
    mP0 = p0; mP1 = p1; mP2 = p2; mP3 = p3;
}

QPointF qCubicSegment2D::posAtT(const qreal &t) const {
    return qPow(1 - t, 3)*p0() +
            3*qPow(1 - t, 2)*t*p1() +
            3*(1 - t)*t*t*p2() +
            t*t*t*p3();
}

qreal qCubicSegment2D::getLength() {
    if(!mLengthUpToDate) updateLength();
    return fLength;
}

qreal qCubicSegment2D::tAtLength(const qreal &length) {
    if(isZero6Dec(length) || length < 0) return 0;
    qreal totLen = getLength();
    if(isZero6Dec(length - totLen) || length > totLen) return 1;
    return tAtLength(length, 0.01, 0, 1);
}

qreal qCubicSegment2D::lengthAtT(qreal t) {
    t = CLAMP(t, 0, 1);
    if(isZero6Dec(qMax(0., t))) return 0;
    if(isZero6Dec(qMin(1., t) - 1)) return getLength();
    auto divSeg = dividedAtT(t);
    return divSeg.first.getLength();
}

qreal qCubicSegment2D::lengthFracAtT(qreal t) {
    t = CLAMP(t, 0, 1);
    if(isZero6Dec(t)) return 0;
    if(isZero6Dec(t - 1)) return getLength();
    qreal totLen = getLength();
    if(isZero6Dec(totLen)) return 1;
    return lengthAtT(t)/totLen;
}

qCubicSegment2D::Pair qCubicSegment2D::dividedAtT(qreal t) {
    t = CLAMP(t, 0, 1);
    qreal oneMinusT = 1 - t;
    QPointF P0_1 = p0()*oneMinusT + p1()*t;
    QPointF P1_2 = p1()*oneMinusT + p2()*t;
    QPointF P2_3 = p2()*oneMinusT + p3()*t;

    QPointF P01_12 = P0_1*oneMinusT + P1_2*t;
    QPointF P12_23 = P1_2*oneMinusT + P2_3*t;

    QPointF P0112_1223 = P01_12*oneMinusT + P12_23*t;

    qCubicSegment2D seg1(p0(), P0_1, P01_12, P0112_1223);
    qCubicSegment2D seg2(P0112_1223, P12_23, P2_3, p3());

    return {seg1, seg2};
}

const QPointF &qCubicSegment2D::p0() const { return mP0; }

const QPointF &qCubicSegment2D::p1() const { return mP1; }

const QPointF &qCubicSegment2D::p2() const { return mP2; }

const QPointF &qCubicSegment2D::p3() const { return mP3; }

void qCubicSegment2D::setP0(const QPointF &p0) { mP0 = p0; }

void qCubicSegment2D::setP1(const QPointF &p1) { mP1 = p1; }

void qCubicSegment2D::setP2(const QPointF &p2) { mP2 = p2; }

void qCubicSegment2D::setP3(const QPointF &p3) { mP3 = p3; }

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
    qreal maxLen = lengthAtT(maxT);//gCubicLength(seg);
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
                qCubicVals;
                while(dist < minError) {
                    bestT = t;
                    bestPt = pt;
                    minError = dist;
                    qreal tMinusOne = t - 1;
                    qreal pow2TMinusOne = pow2(tMinusOne);
                    qreal pow3TMinusOne = pow3(tMinusOne);

                    qreal v0 = 3*p2x - 3*p2x*t + p3x*t;
                    qreal v1 = 3*p2y - 3*p2y*t + p3y*t;

                    qreal v2 = 3*p1x*pow2TMinusOne + t*v0;
                    qreal v3 = t*(3*p1y*pow2TMinusOne + t*v1);

                    qreal v4 = -1 + 4*t - 3*pow2(t);

                    qreal num = pow2(p.x() + p0x*pow3TMinusOne - t*v2) +
                                    pow2(p.y() + p0y*pow3TMinusOne - v3);
                    qreal den = -6*(p0x*pow2TMinusOne + t*(-2*p2x + 3*p2x*t - p3x*t) +
                                    p1x*v4)*
                                  (-p.x() - p0x*pow3TMinusOne +  t*v2) -
                                 6*(p0y*pow2TMinusOne + t*(-2*p2y + 3*p2y*t - p3y*t) +
                                    p1y*v4)*
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

//QList<qrealPair> qCubicSegment2D::sIntersectionTs(
//        qCubicSegment2D &seg1, qCubicSegment2D &seg2) {
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
    path.cubicTo(p1(), p2(), p3());
    fLength = path.length();
}
