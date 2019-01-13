#include "pointhelpers.h"
#include "exceptions.h"

#include <QtMath>
#include <complex>
#include <QDebug>
#include <QMatrix>
#include <QList>
typedef std::complex<double> cmplx;

qCubicSegment1D xSeg(const qCubicSegment2D &seg) {
    return {seg.fP0.x(), seg.fP1.x(), seg.fP2.x(), seg.fP3.x()};
}

qCubicSegment1D ySeg(const qCubicSegment2D &seg) {
    return {seg.fP0.y(), seg.fP1.y(), seg.fP2.y(), seg.fP3.y()};
}

CubicList gPathToQCubicSegs2D(const SkPath& path) {
    CubicList segs;

    SkPoint lastMovePos;
    SkPoint lastPos;
    SkPath::Iter iter(path, false);
    for(;;) {
        SkPoint pts[4];
        switch(iter.next(pts, true, true)) {
            case SkPath::kLine_Verb: {
                const SkPoint& pt1 = pts[1];
                SkCubicSegment2D seg{lastPos, lastPos, pt1, pt1};
                segs << qCubicSegment2D(seg);
                lastPos = pt1;
            } break;
            case SkPath::kQuad_Verb: {
                const SkPoint& pt2 = pts[2];
                SkQuadSegment2D seg{lastPos, pts[1], pt2};
                segs << qQuadSegment2D(seg).toCubic();
                lastPos = pt2;
            } break;
            case SkPath::kConic_Verb: {
                const SkPoint& pt2 = pts[2];
                SkConicSegment2D seg{lastPos, pts[1], pt2, iter.conicWeight()};
                segs << qConicSegment2D(seg).toCubic();
                lastPos = pt2;
            } break;
            case SkPath::kCubic_Verb: {
                const SkPoint& pt3 = pts[3];
                SkCubicSegment2D seg{lastPos, pts[1], pts[2], pt3};
                segs << qCubicSegment2D(seg);
                lastPos = pt3;
            } break;
            case SkPath::kClose_Verb: {
//                SkCubicSegment2D seg{lastPos, lastPos, lastMovePos, lastMovePos};
//                segs << qCubicSegment2D(seg);
            } break;
            case SkPath::kMove_Verb: {
                lastMovePos = pts[0];
                lastPos = lastMovePos;
            } break;
            case SkPath::kDone_Verb:
                return segs;
        }
    }
}

QPointF symmetricToPos(QPointF toMirror,
                       QPointF mirrorCenter) {
    QPointF posDist = toMirror - mirrorCenter;
    return mirrorCenter - posDist;
}

QPointF symmetricToPosNewLen(QPointF toMirror,
                             QPointF mirrorCenter,
                             qreal newLen) {
    QPointF posDist = toMirror - mirrorCenter;
    return mirrorCenter - scalePointToNewLen(posDist, newLen);
}

QPointF gCubicValueAtT(const qCubicSegment2D &seg,
                           const qreal &t) {
    return QPointF(gCubicValueAtT(xSeg(seg), t),
                   gCubicValueAtT(ySeg(seg), t));
}

qreal gCubicValueAtT(const qCubicSegment1D &seg,
                         const qreal& t) {
    return qPow(1 - t, 3)*seg.fP0 +
            3*qPow(1 - t, 2)*t*seg.fP1 +
            3*(1 - t)*t*t*seg.fP2 +
            t*t*t*seg.fP3;
}

qreal gSolveForP2(const qreal& p0, const qreal& p1,
                  const qreal& p3, const qreal& t,
                  const qreal& value) {
    if(isZero4Dec(t*t - t)) RuntimeThrow("Cannot solve with t*t == t.");
    qreal tm1 = t - 1.;
    return -(p0*tm1*tm1*tm1 - 3.*p1*t*tm1*tm1 - p3*t*t*t + value)/(3*tm1*t*t);
}

qreal gSolveForP1(const qreal& p0, const qreal& p2,
                  const qreal& p3, const qreal& t,
                  const qreal& value) {
    if(isZero4Dec(t*t - t)) RuntimeThrow("Cannot solve with t*t == t.");
    qreal tm1 = t - 1.;
    return (p0*tm1*tm1*tm1 + 3.*p2*t*t*tm1 - p3*t*t*t + value)/(3*tm1*tm1*t);
}

// only for beziers that do not have multiple points of the same x value
// for qrealanimators
qreal gTFromX(const qCubicSegment1D &seg,
             const qreal& x) {
    qreal minT = 0.;
    qreal maxT = 1.;
    qreal xGuess;
    qreal guessT;
    do {
        guessT = (maxT + minT)*0.5;
        xGuess = gCubicValueAtT(seg, guessT);
        if(xGuess > x) {
            maxT = guessT;
        } else {
            minT = guessT;
        }
    } while(qAbs(xGuess - x) > 0.0001);
    return guessT;
}

void gGetCtrlsSymmetricPos(const QPointF& endPos,
                           const QPointF& startPos,
                           const QPointF& centerPos,
                           QPointF &newEndPos,
                           QPointF &newStartPos) {
    QPointF symStartPos = symmetricToPos(endPos, centerPos);
    qreal len1 = pointToLen(symStartPos);
    qreal len2 = pointToLen(startPos);
    qreal lenSum = len1 + len2;
    newStartPos = (symStartPos*len1 + startPos*len2)/lenSum;
    newEndPos = symmetricToPos(newStartPos, centerPos);
}

void gGetCtrlsSmoothPos(const QPointF& endPos,
                        const QPointF&startPos,
                        const QPointF& centerPos,
                        QPointF &newEndPos,
                        QPointF &newStartPos) {
    QPointF symEndPos = symmetricToPos(endPos, centerPos);
    qreal len1 = pointToLen(symEndPos);
    qreal len2 = pointToLen(startPos);
    qreal lenSum = len1 + len2;
    QPointF point1Rel = endPos - centerPos;
    QPointF point2Rel = startPos - centerPos;
    QPointF newStartDirection =
            scalePointToNewLen(
                (symEndPos*len1 + startPos*len2)/lenSum - centerPos, 1.);
    qreal startCtrlPtLen =
            qAbs(QPointF::dotProduct(point2Rel, newStartDirection));
    newStartPos = newStartDirection*startCtrlPtLen + centerPos;
    qreal endCtrlPtLen =
            qAbs(QPointF::dotProduct(point1Rel, newStartDirection));
    newEndPos = -newStartDirection*endCtrlPtLen + centerPos;
}

void solveClosestToSegment(const qCubicSegment1D &seg,
                           const qreal &vn,
                           QList<qreal> *list) {
    cmplx v0 = cmplx(seg.fP0, 0.);
    cmplx v1 = cmplx(seg.fP1, 0.);
    cmplx v2 = cmplx(seg.fP2, 0.);
    cmplx v3 = cmplx(seg.fP3, 0.);
    cmplx v = cmplx(vn, 0.);
    cmplx var1 = sqrt(pow(v1, 2.) + pow(v2, 2.) - v1*(v2 + v3) -
                                 v0*(v3 - v2));
    cmplx var2 = v0 - 3.*v1 + 3.*v2 - v3;
    cmplx var3 = v0 - 2.*v1 + v2;
    cmplx sol1 = (var1 - var3)/-var2;
    cmplx sol2 = (var1 + var3)/var2;

    cmplx var4 =
            pow(-2.*pow(v1,3.) + 3.*v0*v1*v2 + 3.*pow(v1,2.)*v2 -
              6.*v0*pow(v2,2.) + 3.*v1*pow(v2,2.) - 2.*pow(v2,3.) +
              v*pow(v0 - 3.*v1 + 3.*v2 - v3,2.) - pow(v0,2.)*v3 + 3.*v0*v1*v3 -
              6.*pow(v1,2.)*v3 + 3.*v0*v2*v3 + 3.*v1*v2*v3 - v0*pow(v3,2.) +
              sqrt(pow(v0 - 3.*v1 + 3.*v2 - v3,2.)*
                (-3.*pow(v1,2.)*pow(v2,2.) + 4.*v0*pow(v2,3.) +
                  pow(v,2.)*pow(v0 - 3.*v1 + 3.*v2 - v3,2.) + 4.*pow(v1,3.)*v3 -
                  6.*v0*v1*v2*v3 + pow(v0,2.)*pow(v3,2.) -
                  2.*v*(2.*pow(v1,3.) + 2.*pow(v2,3.) -
                     3.*pow(v1,2.)*(v2 - 2.*v3) + pow(v0,2.)*v3 -
                     3.*v1*(v0 + v2)*(v2 + v3) +
                     v0*(6.*pow(v2,2.) - 3.*v2*v3 + pow(v3,2.))))),
             1./3.);

    cmplx sol3 = (-2.*(v0 - 2.*v1 + v2) + (2.*v2to1div3*
                    (pow(v1,2.) + pow(v2,2.) + v0*(-v2 + v3) - v1*(v2 + v3)))/
                  var4 + v2to2div3*
                  var4)/(2.*(-v0 + 3.*v1 - 3.*v2 + v3));
    cmplx sol4 =
            (-36.*(v0 - 2.*v1 + v2) - (cmplx(0.,18.)*v2to1div3*
                     (cmplx(0.,-1.) + sqrt(3.))*
                     (pow(v1,2.) + pow(v2,2.) + v0*(-v2 + v3) - v1*(v2 + v3)))/
                   var4 + cmplx(0,9)*v2to2div3*
                   (cmplx(0.,1.) + sqrt(3.))*
                   var4)/(36.*(-v0 + 3.*v1 - 3.*v2 + v3));

    cmplx sol5 =
            (-36.*(v0 - 2.*v1 + v2) + (cmplx(0.,18.)*v2to1div3*
           (cmplx(0.,1.) + sqrt(3.))*
           (pow(v1,2.) + pow(v2,2.) + v0*(-v2 + v3) - v1*(v2 + v3)))/
         var4 - 9.*v2to2div3*
         (1. + cmplx(0.,1.)*sqrt(3.))*var4)/
            (36.*(-v0 + 3.*v1 - 3.*v2 + v3));
    if(sol1.real() > 0. && sol1.real() < 1.) {
        list->append(sol1.real());
    }
    if(sol2.real() > 0. && sol2.real() < 1.) {
        list->append(sol2.real());
    }
    if(sol3.real() > 0. && sol3.real() < 1.) {
        list->append(sol3.real());
    }
    if(sol4.real() > 0. && sol4.real() < 1.) {
        list->append(sol4.real());
    }
    if(sol5.real() > 0. && sol5.real() < 1.) {
        list->append(sol5.real());
    }
    list->append(0.);
    list->append(1.);
}

void solveOnSegmnet(const qCubicSegment1D &seg,
                    const qreal &xn,
                    QList<qreal> *list) {
    cmplx i = cmplx(0., 1.);
    cmplx x0 = cmplx(seg.fP0, 0.);
    cmplx x1 = cmplx(seg.fP1, 0.);
    cmplx x2 = cmplx(seg.fP2, 0.);
    cmplx x3 = cmplx(seg.fP3, 0.);
    cmplx x = cmplx(xn, 0.);
    cmplx x0sq = pow(x0, 2.);
    cmplx x1sq = pow(x1, 2.);
    cmplx x2sq = pow(x2, 2.);
    cmplx x3sq = pow(x3, 2.);
    cmplx xsq = pow(x, 2.);
    cmplx x1th = pow(x1, 3.);
    cmplx x2th = pow(x2, 3.);
    cmplx _3x2x3 = 3.*x2*x3;
    cmplx _x0x1 = x0*x1;
    cmplx var1 = pow(x0 - 3.*x1 + 3.*x2 - x3, 2.);
    cmplx var2 = sqrt(var1*(-3.*x1sq*x2sq + 4.*x0*x2th + xsq*var1 +
                                4.*x1th*x3 - 2.*_x0x1*_3x2x3 + x0sq*x3sq -
                                2.*x*(2.*x1th + 2.*x2th - 3.*x1sq*(x2 - 2.*x3) +
                                x0sq*x3 - 3.*x1*(x0 + x2)*(x2 + x3) +
                                x0*(6.*x2sq - _3x2x3 + x3sq))) );
    cmplx var3 = pow(-2.*x1th + 3.*_x0x1*x2 + 3.*x1sq*x2 -
                                6.*x0*x2sq + 3.*x1*x2sq - 2.*x2th + x*var1 -
                                x0sq*x3 + 3.*_x0x1*x3 - 6.*x1sq*x3 + x0*_3x2x3 +
                                x1*_3x2x3 - x0*x3sq + var2, 1./3.);
    cmplx var4 = x1sq + x2sq + x0*(-x2 + x3) - x1*(x2 + x3);
    cmplx var5 = -x0 + 3.*x1 - 3.*x2 + x3;
    cmplx var6 = x0 - 2.*x1 + x2;
    cmplx t1 = (-var6 + (v2to1div3*var4)/var3 + v2to2div3*var3*0.5) / var5;
    cmplx t2 = (-4.*var6 - (2.*i*v2to1div3*(-i + sqrt3)*var4)/var3 +
                 i*v2to2div3*(i + sqrt3)*var3) / (4.*var5);
    cmplx t3 = (-4.*var6 + (2.*i*v2to1div3*(i + sqrt3)*var4)/var3 -
                 v2to2div3*(1. + i*sqrt3)*var3 ) / (4.*var5);
    if(t1.real() > 0. && t1.real() < 1.) {
        list->append(t1.real());
    }
    if(t2.real() > 0. && t2.real() < 1.) {
        list->append(t2.real());
    }
    if(t3.real() > 0. && t3.real() < 1.) {
        list->append(t3.real());
    }
    list->append(0.);
    list->append(1.);
}

qreal gGetClosestTValueOnBezier(const qCubicSegment1D &seg,
                               const qreal &p,
                               qreal *bestPosPtr,
                               qreal *errorPtr) {
    QList<qreal> values;

    solveClosestToSegment(seg, p, &values);
    qreal bestT = 0.;
    qreal bestPos = seg.fP0;;
    qreal minErrorT = DBL_MAX;
    Q_FOREACH(const qreal &val, values) {
        qreal posT = gCubicValueAtT(seg, val);
        qreal errorT = qAbs(posT - p);
        if(errorT < minErrorT) {
            bestPos = posT;
            minErrorT = errorT;
            bestT = val;
        }
    }
    if(bestPosPtr != nullptr) {
        *bestPosPtr = bestPos;
    }
    if(errorPtr != nullptr) {
        *errorPtr = minErrorT;
    }
    return bestT;
}

qreal gGetClosestTValueOnBezier(const SkCubicSegment2D &seg,
                                const SkPoint &p,
                                SkPoint *bestPosPtr,
                                qreal *errorPtr) {
    QPointF qBestPos;
    qreal t = gGetClosestTValueOnBezier(qCubicSegment2D(seg),
                                       skPointToQ(p),
                                       &qBestPos,
                                       errorPtr);
    if(bestPosPtr) *bestPosPtr = qPointToSk(qBestPos);
    return t;
}
#define qCubicToVals(seg) \
    const qreal& p0x = seg.fP0.x(); \
    const qreal& p0y = seg.fP0.y(); \
    const qreal& p1x = seg.fP1.x(); \
    const qreal& p1y = seg.fP1.y(); \
    const qreal& p2x = seg.fP2.x(); \
    const qreal& p2y = seg.fP2.y(); \
    const qreal& p3x = seg.fP3.x(); \
    const qreal& p3y = seg.fP3.y();

qreal gGetClosestTValueOnBezier(const qCubicSegment2D &seg,
                                const QPointF &p,
                                QPointF *bestPosPtr,
                                qreal *errorPtr) {
    return gGetClosestTValueOnBezier(seg, p, 0, 1, bestPosPtr, errorPtr);
}

qreal gGetClosestTValueOnBezier(const qCubicSegment2D &seg,
                                const QPointF &p,
                                const qreal& minT,
                                const qreal& maxT,
                                QPointF *bestPosPtr,
                                qreal *errorPtr) {
    qreal totalLen = gCubicLengthAtT(seg, maxT);//gCubicLength(seg);
    qreal bestT = 0;
    QPointF bestPt = seg.fP0;
    qreal minError = DBL_MAX;
    for(qreal len = gCubicLengthAtT(seg, minT); len < totalLen;) { // t ∈ [0., 1.]
        qreal t = gCubicTimeAtLength(seg, len);
        QPointF pt = gCubicValueAtT(seg, t);
        qreal dist = pointToLen(pt - p);
        if(dist < minError) {
            bestT = t;
            bestPt = pt;
            minError = dist;
            if(minError < 1) {
                qCubicToVals(seg);
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
                    pt = gCubicValueAtT(seg, newT);
                    dist = pointToLen(pt - p);
                    t = newT;
                }
            }
            if(minError < 0.01) break;
        }

        len += dist*0.8;
    }
    if(bestPosPtr != nullptr) {
        *bestPosPtr = bestPt;
    }
    if(errorPtr != nullptr) {
        *errorPtr = minError;
    }
    return bestT;
}

//void VectorPathAnimator::bezierLeastSquareV1V2(const qreal &v0,
//                                               qreal &v1, qreal &v2,
//                                               const qreal &v3,
//                                               const QList<qreal> &vs,
//                                               const QList<qreal> &ts) {
//    qreal v1Inc = 0.;
//    qreal v1Dec = 0.;
//    qreal v2Inc = 0.;
//    qreal v2Dec = 0.;
//    for(int i = 0; i < vs.count(); i++) {
//        const qreal &val = vs.at(i);
//        const qreal &t = ts.at(i);
//        v1Inc += pow(1. - t, 2.)*t*(-v0*pow(1. - t, 3.) - 3.*v2*(1. - t)*pow(t, 2.) -
//                 v3*pow(t, 3.) + val);
//        v1Dec += (1. - t)*pow(t, 2.)*(3.*t*pow(1. - t, 2.));
//        v2Inc += (1. - t)*pow(t, 2.)*(-v0*pow(1. - t, 3.) - 3.*v1*pow(1. - t, 2.)*t -
//                 v3*pow(t, 3.) + val);
//        v2Dec += (1. - t)*pow(t, 2.)*(3.*(1. - t)*pow(t, 2.));
//    }
//    v1 = v1Inc/v1Dec;
//    v2 = v2Inc/v2Dec;
//}

qCubicSegment2D gBezierLeastSquareV1V2(const qCubicSegment2D &seg,
                                       const QList<QPointF> &vs,
                                       const QList<qreal> &ts) {
    qreal v1XInc = 0.;
    qreal v1Dec = 0.;
    qreal v2XInc = 0.;
    qreal v2Dec = 0.;
    qreal v1YInc = 0.;
    qreal v2YInc = 0.;
    for(int i = 0; i < vs.count(); i++) {
        const QPointF &val = vs.at(i);
        const qreal &t = ts.at(i);
        v1XInc += pow(1. - t, 2.)*t*(-seg.fP0.x()*pow(1. - t, 3.) - 3.*seg.fP2.x()*(1. - t)*pow(t, 2.) -
                 seg.fP3.x()*pow(t, 3.) + val.x());
        v2XInc += (1. - t)*pow(t, 2.)*(-seg.fP0.x()*pow(1. - t, 3.) - 3.*seg.fP1.x()*pow(1. - t, 2.)*t -
                 seg.fP3.x()*pow(t, 3.) + val.x());
        v1YInc += pow(1. - t, 2.)*t*(-seg.fP0.y()*pow(1. - t, 3.) - 3.*seg.fP2.y()*(1. - t)*pow(t, 2.) -
                 seg.fP3.y()*pow(t, 3.) + val.y());
        v2YInc += (1. - t)*pow(t, 2.)*(-seg.fP0.y()*pow(1. - t, 3.) - 3.*seg.fP1.y()*pow(1. - t, 2.)*t -
                 seg.fP3.y()*pow(t, 3.) + val.y());
        v1Dec = (1. - t)*pow(t, 2.)*(3.*t*pow(1. - t, 2.));
        v2Dec = (1. - t)*pow(t, 2.)*(3.*(1. - t)*pow(t, 2.));
    }
    qCubicSegment2D newSeg;
    newSeg.fP0 = seg.fP0;
    newSeg.fP1 = QPointF(v1XInc/v1Dec, v1YInc/v1Dec);
    newSeg.fP2 = QPointF(v2XInc/v2Dec, v2YInc/v2Dec);
    newSeg.fP3 = seg.fP3;
    return newSeg;
}

qCubicSegment2D gBezierLeastSquareV1V2(
        const qCubicSegment2D &seg,
        const QList<QPointF> &vs,
        const int &minVs,
        const int &maxVs) {
    QPointF v1, v2;
    for(int j = 0; j < 50; j++) {
        qreal v1XInc = 0.;
        qreal v1Dec = 0.;
        qreal v2XInc = 0.;
        qreal v2Dec = 0.;
        qreal v1YInc = 0.;
        qreal v2YInc = 0.;
        for(int i = minVs; i <= maxVs; i++) {
            const QPointF &val = vs.at(i);
            qreal t = (static_cast<qreal>(i) - minVs)/(maxVs - minVs);
            //qreal t = getClosestTValueBezier2D(seg.p0, seg.p1, seg.p2, seg.p3, val);
            v1XInc += pow(1. - t, 2.)*t*(-seg.fP0.x()*pow(1. - t, 3.) - 3.*seg.fP2.x()*(1. - t)*pow(t, 2.) -
                     seg.fP3.x()*pow(t, 3.) + val.x());
            v2XInc += (1. - t)*pow(t, 2.)*(-seg.fP0.x()*pow(1. - t, 3.) - 3.*seg.fP1.x()*pow(1. - t, 2.)*t -
                     seg.fP3.x()*pow(t, 3.) + val.x());
            v1YInc += pow(1. - t, 2.)*t*(-seg.fP0.y()*pow(1. - t, 3.) - 3.*seg.fP2.y()*(1. - t)*pow(t, 2.) -
                     seg.fP3.y()*pow(t, 3.) + val.y());
            v2YInc += (1. - t)*pow(t, 2.)*(-seg.fP0.y()*pow(1. - t, 3.) - 3.*seg.fP1.y()*pow(1. - t, 2.)*t -
                     seg.fP3.y()*pow(t, 3.) + val.y());
            v1Dec += pow(1. - t, 2.)*t*(3.*t*pow(1. - t, 2.));
            v2Dec += (1. - t)*pow(t, 2.)*(3.*(1. - t)*pow(t, 2.));
        }
        v1 = QPointF(v1XInc/v1Dec, v1YInc/v1Dec);
        v2 = QPointF(v2XInc/v2Dec, v2YInc/v2Dec);
    }
    return {seg.fP0, v1, v2, seg.fP3};
}

qreal get1DAccuracyValue(const qreal &x0,
                         const qreal &x1,
                         const qreal &x2,
                         const qreal &x3) {
    return qMax4(x0, x1, x2, x3) - qMin4(x0, x1, x2, x3);
}

QPointF gGetClosestPointOnLineSegment(const QPointF &a,
                                      const QPointF &b,
                                      const QPointF &p) {
    QPointF AP(p - a); //Vector from A to P
    QPointF AB(b - a); //Vector from A to B

    qreal magnitudeAB = pow2(AB.x()) + pow2(AB.y()); //Magnitude of AB vector (it's length squared)
    qreal ABAPproduct = AP.x()*AB.x() + AP.y()*AB.y(); //The DOT product of a_to_p and a_to_b
    qreal distance = ABAPproduct / magnitudeAB; //The normalized "distance" from a to your closest point

    if(distance < 0) { //Check if P projection is over vectorAB
        return a;
    } else if(distance > 1) {
        return b;
    } else {
        return a + (AB * distance);
    }
}

QPointF gClosestPointOnRect(const QRectF &rect,
                           const QPointF &point,
                           qreal *dist) {
    qreal minDist = DBL_MAX;
    QPointF bestPos;
    if(point.y() > rect.bottom()) {
        // check bottom
        QPointF pt = gGetClosestPointOnLineSegment(rect.bottomLeft(),
                                                  rect.bottomRight(),
                                                  point);
        qreal dist = pointToLen(pt - point);
        if(dist < minDist) {
            minDist = dist;
            bestPos = pt;
        }
    } else if(point.y() < rect.top()) {
        // check top
        QPointF pt = gGetClosestPointOnLineSegment(rect.topLeft(),
                                                  rect.topRight(),
                                                  point);
        qreal dist = pointToLen(pt - point);
        if(dist < minDist) {
            minDist = dist;
            bestPos = pt;
        }
    }

    if(point.x() > rect.right()) {
        // check right
        QPointF pt = gGetClosestPointOnLineSegment(rect.topRight(),
                                                  rect.bottomRight(),
                                                  point);
        qreal dist = pointToLen(pt - point);
        if(dist < minDist) {
            minDist = dist;
            bestPos = pt;
        }
    } else if(point.y() < rect.left()) {
        // check left
        QPointF pt = gGetClosestPointOnLineSegment(rect.bottomLeft(),
                                                  rect.topLeft(),
                                                  point);
        qreal dist = pointToLen(pt - point);
        if(dist < minDist) {
            minDist = dist;
            bestPos = pt;
        }
    }
    if(dist != nullptr) *dist = minDist;
    return bestPos;
}

qreal gGetBezierTValueForX(const qCubicSegment1D &seg,
                          const qreal &x,
                          qreal *error) {
    if(qAbs(seg.fP0 - x) < 0.01) return seg.fP0;
    if(qAbs(seg.fP3 - x) < 0.01) return seg.fP3;

    QList<qreal> xValues;

    solveOnSegmnet(seg, x, &xValues);
    qreal bestT = 0.;
    qreal minErrorT = 1000000.;

    Q_FOREACH(const qreal &xVal, xValues) {
        qreal errorT = qAbs(gCubicValueAtT(seg, xVal) - x);
        if(errorT < minErrorT) {
            minErrorT = errorT;
            bestT = xVal;
        }
    }

    if(error != nullptr) *error = minErrorT;

    return bestT;
}

qreal gGetBezierTValueForXAssumeNoOverlapGrowingOnly(
        const qCubicSegment1D &seg,
        const qreal &x,
        const qreal &minT, const qreal &maxT,
        const qreal &maxError, qreal *error) {
    qreal tGuess = (maxT + minT)*0.5;
    qreal guessVal = gCubicValueAtT(seg, tGuess);
    qreal errorT = qAbs(guessVal - x);
    if(errorT < maxError) {
        if(error != nullptr) {
            *error = errorT;
        }
        return tGuess;
    }
    if(guessVal > x) {
        return gGetBezierTValueForXAssumeNoOverlapGrowingOnly(seg, x,
                                           minT, tGuess, maxError, error);
    }
    return gGetBezierTValueForXAssumeNoOverlapGrowingOnly(seg, x,
                                       tGuess, maxT, maxError, error);
}

qreal gGetBezierTValueForXAssumeNoOverlapGrowingOnly(
        const qCubicSegment1D &seg,
        const qreal &x,
        const qreal &maxError, qreal *error) {
    if(seg.fP0 > seg.fP3) {
        return gGetBezierTValueForXAssumeNoOverlapGrowingOnly(seg, x,
                                                             maxError, error);
    }
    if(qAbs(seg.fP0 - x) < 0.01) return seg.fP0;
    if(qAbs(seg.fP3 - x) < 0.01) return seg.fP3;
    return gGetBezierTValueForXAssumeNoOverlapGrowingOnly(seg,
                                                         x, 0., 1., maxError,
                                                         error);
}

void gDrawCosmeticEllipse(QPainter *p,
                         const QPointF &absPos,
                         qreal rX, qreal rY) {
    const QTransform &transform = p->transform();
    p->drawEllipse(absPos,
                   rX/transform.m11(),
                   rY/transform.m22());
}

qreal gMinDistanceToPath(const SkPoint &pos, const SkPath &path) {
    QPointF qPos = skPointToQ(pos);
    qreal smallestDist = __DBL_MAX__;
    CubicList segs = gPathToQCubicSegs2D(path);
    foreach(const auto& seg, segs) {
        qreal thisDist;
        gGetClosestTValueOnBezier(seg, qPos, nullptr, &thisDist);
        if(thisDist < smallestDist) smallestDist = thisDist;
    }
    return smallestDist;
}

qreal gCubicLength(const qCubicSegment2D& seg) {
    QPainterPath path;
    path.moveTo(seg.fP0);
    path.cubicTo(seg.fP1, seg.fP2, seg.fP3);
    return path.length();
}

CubicPair gDivideCubicAtT(const qCubicSegment2D& seg,
                          qreal t) {
    t = CLAMP(t, 0, 1);
    qreal oneMinusT = 1 - t;
    QPointF P0_1 = seg.fP0*oneMinusT + seg.fP1*t;
    QPointF P1_2 = seg.fP1*oneMinusT + seg.fP2*t;
    QPointF P2_3 = seg.fP2*oneMinusT + seg.fP3*t;

    QPointF P01_12 = P0_1*oneMinusT + P1_2*t;
    QPointF P12_23 = P1_2*oneMinusT + P2_3*t;

    QPointF P0112_1223 = P01_12*oneMinusT + P12_23*t;

    qCubicSegment2D seg1{seg.fP0, P0_1, P01_12, P0112_1223};
    qCubicSegment2D seg2{P0112_1223, P12_23, P2_3, seg.fP3};

    return {seg1, seg2};
}


qreal gCubicLengthAtT(const qCubicSegment2D& seg, qreal t) {
    t = CLAMP(t, 0, 1);
    if(isZero6Dec(qMax(0., t))) return 0;
    if(isZero6Dec(qMin(1., t) - 1)) return gCubicLength(seg);
    auto divSeg = gDivideCubicAtT(seg, t);
    return gCubicLength(divSeg.first);
}

qreal gCubicLengthFracAtT(const qCubicSegment2D& seg, qreal t) {
    t = CLAMP(t, 0, 1);
    if(isZero6Dec(t)) return 0;
    if(isZero6Dec(t - 1)) return gCubicLength(seg);
    qreal totLen = gCubicLength(seg);
    if(isZero6Dec(totLen)) return 1;
    return gCubicLengthAtT(seg, t)/totLen;
}

qreal cubicTimeAtLength(const qCubicSegment2D& seg,
                        const qreal& length, const qreal& maxLenErr,
                        const qreal& minT, const qreal& maxT) {
    qreal guessT = (maxT + minT)*0.5;
    qreal lenAtGuess = gCubicLengthAtT(seg, guessT);
    if(abs(lenAtGuess - length) < maxLenErr) return guessT;
    if(lenAtGuess > length) {
        return cubicTimeAtLength(seg, length, maxLenErr, minT, guessT);
    } else {
        return cubicTimeAtLength(seg, length, maxLenErr, guessT, maxT);
    }
}

qreal gCubicTimeAtLength(const qCubicSegment2D& seg,
                         const qreal& length) {
    if(isZero6Dec(length) || length < 0) return 0;
    qreal totLen = gCubicLength(seg);
    if(isZero6Dec(length - totLen) || length > totLen) return 1;
    return cubicTimeAtLength(seg, length, 0.01, 0, 1);
}

//! @brief Loop through the segment length using length/div intervals.
//! Returns length increment that was actually used.
qreal fixedDivSegmentLengthLoop(
        const qCubicSegment2D& seg, const uint &div,
        const bool& closedInterval,
        const std::function<void(const qreal&)> &func,
        const qreal& totalLen) {
    if(closedInterval) func(0.);
    for(uint i = 1; i < div; i++) { // t ∈ (0., 1.)
        func(gCubicTimeAtLength(seg, i*totalLen/div));
    }
    if(closedInterval) func(1.);
    return totalLen/div;
}

//! @brief Loop through the segment length using length/div intervals.
//! Returns length increment that was actually used.
qreal gFixedDivSegmentLengthLoop(
        const qCubicSegment2D& seg, const uint &div,
        const bool& closedInterval,
        const std::function<void(const qreal&)> &func) {
    return fixedDivSegmentLengthLoop(seg, div, closedInterval,
                                     func, gCubicLength(seg));
}

//! @brief Loop through the segment using length intervals.
//! Returns length increment that was actually used.
qreal gFixedIncSegmentLengthLoop(
        const qCubicSegment2D& seg, const qreal &lenInc,
        const bool& closedInterval,
        const std::function<void(const qreal&)> &func) {
    if(lenInc < 0 || isZero6Dec(lenInc)) return 0;
    qreal totalLen = gCubicLength(seg);
    uint div = static_cast<uint>(qCeil(totalLen/lenInc));
    return fixedDivSegmentLengthLoop(seg, div, closedInterval,
                                     func, totalLen);
}

//! @brief Loop through the segment using length intervals
//! starting with t = 0, and ending with t = 1.
//! Returns length increment that was actually used.
void gVaryingIncSegmentLengthLoop(
        const qCubicSegment2D& seg,
        const std::function<qreal()> &lenInc,
        const std::function<void(const qreal&)> &func) {
    qreal totalLen = gCubicLength(seg);
    func(0.);
    for(qreal len = lenInc(); len < totalLen; len += lenInc()) { // t ∈ (0., 1.)
        func(gCubicTimeAtLength(seg, len));
    }
    func(1.);
}

#define qCubic1DToNamedVals(seg, add) \
    const qreal& p##add##0 = seg.fP0; \
    const qreal& p##add##1 = seg.fP1; \
    const qreal& p##add##2 = seg.fP2; \
    const qreal& p##add##3 = seg.fP3;

QList<qrealPair> gIntersectionTs(const qCubicSegment2D& seg1,
                                 const qCubicSegment2D& seg2) {
    QList<qrealPair> sols;
    qreal totalLen1 = gCubicLength(seg1);
    qreal totalLen2 = gCubicLength(seg1);

    for(qreal len1 = 0; len1 < totalLen1;) { // t ∈ (0., 1.)
        qreal t1 = gCubicTimeAtLength(seg1, len1);
        QPointF pt1 = gCubicValueAtT(seg1, t1);

        qreal smallestDist = DBL_MAX;
        for(qreal len2 = 0; len2 < totalLen2;) { // t ∈ (0., 1.)
            qreal t2 = gCubicTimeAtLength(seg2, len2);
            QPointF pt2 = gCubicValueAtT(seg2, t2);
            qreal dist = pointToLen(pt1 - pt2);
            if(dist < smallestDist) smallestDist = dist;

            if(dist < .5) {
                sols.append({t1, t2});
                len2 += 1.;
                smallestDist += 1;
                break;
            }

            len2 += dist*0.5;
        }

        len1 += smallestDist*0.5;
    }

    return sols;
}

QList<QPointF> gIntersectionPts(const qCubicSegment2D& seg1,
                                const qCubicSegment2D& seg2) {
    auto tPairs = gIntersectionTs(seg1, seg2);
}

void gSeperateIntersectionTs(const qCubicSegment2D& seg1,
                             const qCubicSegment2D& seg2,
                             QList<qreal>& seg1Ts,
                             QList<qreal>& seg2Ts) {
    qreal totalLen1 = gCubicLength(seg1);
    //qreal totalLen2 = gCubicLength(seg1);

    for(qreal len1 = 0; len1 < totalLen1;) { // t ∈ (0., 1.)
        qreal t1 = gCubicTimeAtLength(seg1, len1);
        QPointF pt1 = gCubicValueAtT(seg1, t1);

        qreal dist;
        QPointF pt2;
        qreal t2 = gGetClosestTValueOnBezier(seg2, pt1, &pt2, &dist);
        if(dist < 0.01) {
            if(!isZeroOrOne6Dec(t1) && !isZeroOrOne6Dec(t2)) {
                seg1Ts << t1;
                seg2Ts << t2;
            }
            len1 += 1;
        }

        len1 += dist*0.8;
    }
}

//! @brief Splits segment at provided t values.
//! Provided list of t values must be sorted.
//! Returns list of segments.
CubicList gSplitAtTs(const qCubicSegment2D& seg,
                     const QList<qreal>& segTs) {
    CubicList segList;
    qCubicSegment2D segRem = seg;
    qreal lastT = 0.;
    foreach(const auto& seg1T, segTs) {
        if(isZeroOrOne6Dec(seg1T)) continue;
        qreal currT = (seg1T - lastT)/(1 - lastT);
        auto divRes = gDivideCubicAtT(segRem, currT);
        segList << divRes.first;
        segRem = divRes.second;
        lastT = currT;
    }
    segList << segRem;
    return segList;
}

CubicList gSplitSelfAtIntersections(const qCubicSegment2D& seg) {
    QList<qreal> segTs;
    if(segTs.isEmpty()) return CubicList() << seg;
    std::sort(segTs.begin(), segTs.end());
    return gSplitAtTs(seg, segTs);
}

CubicListPair gSplitAtIntersections(const qCubicSegment2D& seg1,
                                    const qCubicSegment2D& seg2) {
    QList<qreal> seg1Ts, seg2Ts;
    gSeperateIntersectionTs(seg1, seg2, seg1Ts, seg2Ts);
    std::sort(seg1Ts.begin(), seg1Ts.end());
    std::sort(seg2Ts.begin(), seg2Ts.end());
    CubicList seg1List = gSplitAtTs(seg1, seg1Ts);
    CubicList seg2List = gSplitAtTs(seg2, seg2Ts);

    return {seg1List, seg2List};
}

SkPath gCubicListToSkPath(const CubicList& list) {
    if(list.isEmpty()) return SkPath();
    SkPath path;
    bool first = true;
    QPointF lastPos;
    QPointF firstPos;
    foreach(const auto& cubic, list) {
        if(first) {
            first = false;
            firstPos = cubic.fP0;
            path.moveTo(qPointToSk(firstPos));
        } else if(pointToLen(cubic.fP0 - lastPos) > 0.1)  {
            firstPos = cubic.fP0;
            path.moveTo(qPointToSk(firstPos));
        }
        path.cubicTo(qPointToSk(cubic.fP1),
                     qPointToSk(cubic.fP2),
                     qPointToSk(cubic.fP3));
        lastPos = cubic.fP3;
    }
    return path;
}

CubicList gCubicIntersectList(CubicList targetList) {
    for(int i = 0; i < targetList.count(); i++) {
        qCubicSegment2D iCubic = targetList.at(i);
        for(int j = 0; j < targetList.count(); j++) {
            bool same = i == j;
            qCubicSegment2D jCubic = targetList.at(j);
            CubicListPair inter;
            if(same) {
                inter = {gSplitSelfAtIntersections(iCubic), CubicList()};
            } else {
                inter = gSplitAtIntersections(iCubic, jCubic);
            }

            bool foundInter = inter.first.count() > 1;
            if(foundInter) {
                targetList.removeAt(i--);
                if(j > i) j--;

                for(int k = 0; k < inter.first.count(); k++) {
                    targetList.insert(++i, inter.first.at(k));
                    if(j >= i) j++;
                }
                if(!same) {
                    targetList.removeAt(j--);
                    if(i > j) i--;
                    for(int k = 0; k < inter.second.count(); k++) {
                        targetList.insert(++j, inter.second.at(k));
                        if(i >= j) i++;
                    }
                }

                break;
            }
            if(foundInter) break;
        }

    }
    return targetList;
}

QPointF rotPt(const QPointF& pt, const qreal& deg) {
    return {pt.x() * cos(deg*PI/180) - pt.y() * sin(deg*PI/180),
            pt.x() * sin(deg*PI/180) + pt.y() * cos(deg*PI/180)};
}

qCubicSegment2D gCubicRotate(const qCubicSegment2D &seg,
                             const qreal& deg) {
    if(isZero6Dec(deg)) return seg;
    return { rotPt(seg.fP0, deg), rotPt(seg.fP1, deg),
             rotPt(seg.fP2, deg), rotPt(seg.fP3, deg) };
}

// -90 is y direction 0 is x direction
qreal gCubicGetTFurthestInDirection(const qCubicSegment2D &seg,
                                    const qreal& deg) {
    auto rotSeg = gCubicRotate(seg, deg);
    auto xS = xSeg(rotSeg);
    qCubic1DToNamedVals(xS, x);
    bool p0Further = px0 >= px1 && px0 >= px2;
    bool p3Further = px3 >= px1 && px3 >= px2;
    if(p0Further || p3Further) {
        if(px3 > px0) return 1;
        else return 0;
    }
    qreal den = px0 - 3*px1 + 3*px2 - px3;
    qreal num0 = px0 - 2*px1 + px2;
    qreal numSqrt = sqrt(pow2(px1) - px0*px2 - px1*px2 +
                         pow2(px2) + px0*px3 - px1*px3);
    qreal t1 = (num0 + numSqrt)/den;
    qreal t2 = (num0 - numSqrt)/den;
    if(t1 > 0 || t1 < 1) return t1;
    if(t2 > 0 || t2 < 1) return t2;
    Q_ASSERT(false);
    return 0;
}

bool gCubicListClockWise(const CubicList &list) {
    qreal sum = 0;
    foreach(const auto& seg, list) {
        sum += (seg.fP3.x() - seg.fP0.x())*(seg.fP3.y() + seg.fP0.y());
    }
    return sum > 0;
}

QList<SkPath> gSolidifyCubicList(const CubicList &list,
                                 const qreal& width) {
    QList<SkPath> result;
    foreach(const auto& cubic, list) {
        SkStroke strokerSk;
        strokerSk.setJoin(SkPaint::kRound_Join);
        strokerSk.setCap(SkPaint::kRound_Cap);
        strokerSk.setWidth(static_cast<SkScalar>(qAbs(width*2)));
        SkPath outline;
        strokerSk.strokePath(gCubicToSkPath(cubic), &outline);

        result << outline;
    }
    return result;
}

SkPath gCubicToSkPath(const qCubicSegment2D &seg) {
    SkPath path;
    path.moveTo(qPointToSk(seg.fP0));
    path.cubicTo(qPointToSk(seg.fP1),
                 qPointToSk(seg.fP2),
                 qPointToSk(seg.fP3));
    return path;
}

void gGetSmoothAbsCtrlsForPtBetween(
        const SkPoint &lastP,
        const SkPoint &currP,
        const SkPoint &nextP,
        SkPoint &c1, SkPoint &c2,
        const SkScalar &smoothLen) {
    SkPoint sLastP = lastP - currP;
    sLastP.setLength(1);
    SkPoint sNextP = nextP - currP;
    sNextP.setLength(1);
    SkPoint vectP = (sLastP + sNextP)*0.5f;
    vectP.set(vectP.y(), -vectP.x());
    if(vectP.dot(lastP - currP) > 0) vectP.negate();

    SkScalar nextDist = (currP - nextP).length()*0.5f;
    if(smoothLen < nextDist) {
        vectP.setLength(smoothLen);
    } else {
        vectP.setLength(nextDist);
    }

    c1 = currP + vectP;

    vectP.negate();
    SkScalar lastDist = (currP - lastP).length()*0.5f;
    if(smoothLen < lastDist) {
        vectP.setLength(smoothLen);
    } else {
        vectP.setLength(lastDist);
    }
    c2 = currP + vectP;
}

void gGetMaxSmoothAbsCtrlsForPtBetween(
        const SkPoint &lastP,
        const SkPoint &currP,
        const SkPoint &nextP,
        SkPoint &c1, SkPoint &c2) {
    SkPoint sLastP = lastP - currP;
    sLastP.setLength(1);
    SkPoint sNextP = nextP - currP;
    sNextP.setLength(1);
    SkPoint vectP = (sLastP + sNextP)*0.5f;
    vectP.set(vectP.y(), -vectP.x());
    if(vectP.dot(lastP - currP) > 0) vectP.negate();

    SkScalar nextDist = (currP - nextP).length()*0.5f;
    vectP.setLength(nextDist);

    c1 = currP + vectP;

    vectP.negate();
    SkScalar lastDist = (currP - lastP).length()*0.5f;
    vectP.setLength(lastDist);
    c2 = currP + vectP;
}

void gSmoothyAbsCtrlsForPtBetween(
        const SkPoint &lastP,
        const SkPoint &currP,
        const SkPoint &nextP,
        SkPoint &c1, SkPoint &c2,
        SkScalar smoothness) {
    if(smoothness > 1) smoothness = 1;
    if(smoothness < -1) smoothness = -1;

    if(smoothness > 0) {
        SkPoint maxC1; SkPoint maxC2;
        gGetMaxSmoothAbsCtrlsForPtBetween(lastP, currP, nextP, maxC1, maxC2);
        c1 = currP + (c1 - currP)*(1 - smoothness) + (maxC1 - currP)*smoothness;
        c2 = currP + (c2 - currP)*(1 - smoothness) + (maxC2 - currP)*smoothness;
    } else {
        SkScalar smoothnessDec = smoothness + 1;
        c1 = currP + (c1 - currP)*smoothnessDec;
        c2 = currP + (c2 - currP)*smoothnessDec;
    }
}

QList<CubicList> gPathToQCubicSegs2DBreakApart(const SkPath& path) {
    QList<CubicList> result;
    CubicList segs;

    SkPoint lastMovePos;
    SkPoint lastPos;
    SkPath::Iter iter(path, false);
    for(;;) {
        SkPoint pts[4];
        switch(iter.next(pts, true, true)) {
            case SkPath::kLine_Verb: {
                const SkPoint& pt1 = pts[1];
                SkCubicSegment2D seg{lastPos, lastPos, pt1, pt1};
                segs << qCubicSegment2D(seg);
                lastPos = pt1;
            } break;
            case SkPath::kQuad_Verb: {
                const SkPoint& pt2 = pts[2];
                SkQuadSegment2D seg{lastPos, pts[1], pt2};
                segs << qQuadSegment2D(seg).toCubic();
                lastPos = pt2;
            } break;
            case SkPath::kConic_Verb: {
                const SkPoint& pt2 = pts[2];
                SkConicSegment2D seg{lastPos, pts[1], pt2, iter.conicWeight()};
                segs << qConicSegment2D(seg).toCubic();
                lastPos = pt2;
            } break;
            case SkPath::kCubic_Verb: {
                const SkPoint& pt3 = pts[3];
                SkCubicSegment2D seg{lastPos, pts[1], pts[2], pt3};
                segs << qCubicSegment2D(seg);
                lastPos = pt3;
            } break;
            case SkPath::kClose_Verb: {
//                SkCubicSegment2D seg{lastPos, lastPos, lastMovePos, lastMovePos};
//                segs << qCubicSegment2D(seg);
            } break;
            case SkPath::kMove_Verb: {
                if(!segs.isEmpty()) {
                    result << segs;
                    segs.clear();
                }
                lastMovePos = pts[0];
                lastPos = lastMovePos;
            } break;
            case SkPath::kDone_Verb: {
                if(!segs.isEmpty()) {
                    result << segs;
                    segs.clear();
                }
                return result;
            }
        }
    }
}

bool cubicListClosed(const CubicList& list) {
    if(list.isEmpty()) return false;
    if(pointToLen(list.first().fP0 - list.last().fP3) < 0.1) return true;
    return false;
}

SkPath gSmoothyPath(const SkPath& path, const SkScalar& smootness) {
    SkPath result;

    auto segLists = gPathToQCubicSegs2DBreakApart(path);
    foreach(const auto& segList, segLists) {
        if(segList.isEmpty()) continue;
        bool closed = cubicListClosed(segList);
        qCubicSegment2D prevSeg;
        SkPoint c1;
        if(closed) {
            prevSeg = segList.last();
            c1 = qPointToSk(prevSeg.fP2);
            result.moveTo(qPointToSk(prevSeg.fP0));
        }

        for(int i = 0; i < segList.count(); i++) {
            const auto& seg = segList.at(i);
            if(!closed) {
                if(i == 0) {
                    prevSeg = seg;
                    c1 = qPointToSk(prevSeg.fP2);
                    result.moveTo(qPointToSk(prevSeg.fP0));
                    continue;
                }
            }
            SkPoint c2 = qPointToSk(seg.fP1);
            gSmoothyAbsCtrlsForPtBetween(qPointToSk(prevSeg.fP0),
                                      qPointToSk(seg.fP0),
                                      qPointToSk(seg.fP3),
                                      c1, c2, smootness);
            result.cubicTo(c1, c2, qPointToSk(seg.fP0));
            c1 = qPointToSk(seg.fP2);
            prevSeg = seg;
        }
        if(!closed) {
            const auto& seg = segList.last();
            SkPoint c2 = qPointToSk(seg.fP3);
            result.cubicTo(c1, c2, qPointToSk(seg.fP3));
        }
    }

    return result;
}
