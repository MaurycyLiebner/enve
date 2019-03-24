#include "pointhelpers.h"
#include "exceptions.h"

#include <QtMath>
#include <complex>
#include <QDebug>
#include <QMatrix>
#include <QList>
typedef std::complex<double> cmplx;

qCubicSegment1D xSeg(const qCubicSegment2D &seg) {
    return {seg.p0().x(), seg.c1().x(), seg.c2().x(), seg.p3().x()};
}

qCubicSegment1D ySeg(const qCubicSegment2D &seg) {
    return {seg.p0().y(), seg.c1().y(), seg.c2().y(), seg.p3().y()};
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
    return qPow(1 - t, 3)*seg.p0() +
            3*qPow(1 - t, 2)*t*seg.c1() +
            3*(1 - t)*t*t*seg.c2() +
            t*t*t*seg.p1();
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

void gGetCtrlsSymmetricPos(const QPointF& startPos,
                           const QPointF& centerPos,
                           const QPointF& endPos,
                           QPointF &newStartPos,
                           QPointF &newEndPos) {
    const QPointF symStartPos = symmetricToPos(endPos, centerPos);
    const qreal len1 = pointToLen(symStartPos);
    const qreal len2 = pointToLen(startPos);
    const qreal lenSum = len1 + len2;
    newStartPos = (symStartPos*len1 + startPos*len2)/lenSum;
    newEndPos = symmetricToPos(newStartPos, centerPos);
}

void gGetCtrlsSmoothPos(const QPointF& startPos,
                        const QPointF& centerPos,
                        const QPointF& endPos,
                        QPointF &newStartPos,
                        QPointF &newEndPos) {
    const QPointF symEndPos = symmetricToPos(endPos, centerPos);
    const qreal len1 = pointToLen(symEndPos);
    const qreal len2 = pointToLen(startPos);
    const qreal lenSum = len1 + len2;
    const QPointF point1Rel = endPos - centerPos;
    const QPointF point2Rel = startPos - centerPos;
    const QPointF newStartDirection =
            scalePointToNewLen(
                (symEndPos*len1 + startPos*len2)/lenSum - centerPos, 1);
    const qreal startCtrlPtLen =
            qAbs(QPointF::dotProduct(point2Rel, newStartDirection));
    newStartPos = newStartDirection*startCtrlPtLen + centerPos;
    const qreal endCtrlPtLen =
            qAbs(QPointF::dotProduct(point1Rel, newStartDirection));
    newEndPos = -newStartDirection*endCtrlPtLen + centerPos;
}

void solveClosestToSegment(const qCubicSegment1D &seg,
                           const qreal &vn,
                           QList<qreal> *list) {
    const cmplx v0 = cmplx(seg.p0(), 0.);
    const cmplx v1 = cmplx(seg.c1(), 0.);
    const cmplx v2 = cmplx(seg.c2(), 0.);
    const cmplx v3 = cmplx(seg.p1(), 0.);
    const cmplx v = cmplx(vn, 0.);
    const cmplx var1 = sqrt(pow(v1, 2.) + pow(v2, 2.) - v1*(v2 + v3) -
                                 v0*(v3 - v2));
    const cmplx var2 = v0 - 3.*v1 + 3.*v2 - v3;
    const cmplx var3 = v0 - 2.*v1 + v2;
    const cmplx sol1 = (var1 - var3)/-var2;
    const cmplx sol2 = (var1 + var3)/var2;

    const cmplx var4 =
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

    const cmplx sol3 = (-2.*(v0 - 2.*v1 + v2) + (2.*v2to1div3*
                    (pow(v1,2.) + pow(v2,2.) + v0*(-v2 + v3) - v1*(v2 + v3)))/
                  var4 + v2to2div3*
                  var4)/(2.*(-v0 + 3.*v1 - 3.*v2 + v3));
    const cmplx sol4 =
            (-36.*(v0 - 2.*v1 + v2) - (cmplx(0.,18.)*v2to1div3*
                     (cmplx(0.,-1.) + sqrt(3.))*
                     (pow(v1,2.) + pow(v2,2.) + v0*(-v2 + v3) - v1*(v2 + v3)))/
                   var4 + cmplx(0,9)*v2to2div3*
                   (cmplx(0.,1.) + sqrt(3.))*
                   var4)/(36.*(-v0 + 3.*v1 - 3.*v2 + v3));

    const cmplx sol5 =
            (-36.*(v0 - 2.*v1 + v2) + (cmplx(0.,18.)*v2to1div3*
           (cmplx(0.,1.) + sqrt(3.))*
           (pow(v1,2.) + pow(v2,2.) + v0*(-v2 + v3) - v1*(v2 + v3)))/
         var4 - 9.*v2to2div3*
         (1. + cmplx(0.,1.)*sqrt(3.))*var4)/
            (36.*(-v0 + 3.*v1 - 3.*v2 + v3));
    if(sol1.real() > 0 && sol1.real() < 1) {
        list->append(sol1.real());
    }
    if(sol2.real() > 0 && sol2.real() < 1) {
        list->append(sol2.real());
    }
    if(sol3.real() > 0 && sol3.real() < 1) {
        list->append(sol3.real());
    }
    if(sol4.real() > 0 && sol4.real() < 1) {
        list->append(sol4.real());
    }
    if(sol5.real() > 0 && sol5.real() < 1) {
        list->append(sol5.real());
    }
    list->append(0);
    list->append(1);
}

void solveOnSegmnet(const qCubicSegment1D &seg,
                    const qreal &xn,
                    QList<qreal> *list) {
    const cmplx i = cmplx(0., 1.);
    const cmplx x0 = cmplx(seg.p0(), 0.);
    const cmplx x1 = cmplx(seg.c1(), 0.);
    const cmplx x2 = cmplx(seg.c2(), 0.);
    const cmplx x3 = cmplx(seg.p1(), 0.);
    const cmplx x = cmplx(xn, 0.);
    const cmplx x0sq = pow(x0, 2.);
    const cmplx x1sq = pow(x1, 2.);
    const cmplx x2sq = pow(x2, 2.);
    const cmplx x3sq = pow(x3, 2.);
    const cmplx xsq = pow(x, 2.);
    const cmplx x1th = pow(x1, 3.);
    const cmplx x2th = pow(x2, 3.);
    const cmplx _3x2x3 = 3.*x2*x3;
    const cmplx _x0x1 = x0*x1;
    const cmplx var1 = pow(x0 - 3.*x1 + 3.*x2 - x3, 2.);
    const cmplx var2 = sqrt(var1*(-3.*x1sq*x2sq + 4.*x0*x2th + xsq*var1 +
                                4.*x1th*x3 - 2.*_x0x1*_3x2x3 + x0sq*x3sq -
                                2.*x*(2.*x1th + 2.*x2th - 3.*x1sq*(x2 - 2.*x3) +
                                x0sq*x3 - 3.*x1*(x0 + x2)*(x2 + x3) +
                                x0*(6.*x2sq - _3x2x3 + x3sq))) );
    const cmplx var3 = pow(-2.*x1th + 3.*_x0x1*x2 + 3.*x1sq*x2 -
                                6.*x0*x2sq + 3.*x1*x2sq - 2.*x2th + x*var1 -
                                x0sq*x3 + 3.*_x0x1*x3 - 6.*x1sq*x3 + x0*_3x2x3 +
                                x1*_3x2x3 - x0*x3sq + var2, 1./3.);
    const cmplx var4 = x1sq + x2sq + x0*(-x2 + x3) - x1*(x2 + x3);
    const cmplx var5 = -x0 + 3.*x1 - 3.*x2 + x3;
    const cmplx var6 = x0 - 2.*x1 + x2;
    const cmplx t1 = (-var6 + (v2to1div3*var4)/var3 + v2to2div3*var3*0.5) / var5;
    const cmplx t2 = (-4.*var6 - (2.*i*v2to1div3*(-i + sqrt3)*var4)/var3 +
                 i*v2to2div3*(i + sqrt3)*var3) / (4.*var5);
    const cmplx t3 = (-4.*var6 + (2.*i*v2to1div3*(i + sqrt3)*var4)/var3 -
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
    qreal bestPos = seg.p0();;
    qreal minErrorT = DBL_MAX;
    for(const qreal &val : values) {
        qreal posT = gCubicValueAtT(seg, val);
        qreal errorT = qAbs(posT - p);
        if(errorT < minErrorT) {
            bestPos = posT;
            minErrorT = errorT;
            bestT = val;
        }
    }
    if(bestPosPtr) {
        *bestPosPtr = bestPos;
    }
    if(errorPtr) {
        *errorPtr = minErrorT;
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
        v1XInc += pow(1. - t, 2.)*t*(-seg.p0().x()*pow(1. - t, 3.) - 3.*seg.c2().x()*(1. - t)*pow(t, 2.) -
                 seg.p3().x()*pow(t, 3.) + val.x());
        v2XInc += (1. - t)*pow(t, 2.)*(-seg.p0().x()*pow(1. - t, 3.) - 3.*seg.c1().x()*pow(1. - t, 2.)*t -
                 seg.p3().x()*pow(t, 3.) + val.x());
        v1YInc += pow(1. - t, 2.)*t*(-seg.p0().y()*pow(1. - t, 3.) - 3.*seg.c2().y()*(1. - t)*pow(t, 2.) -
                 seg.p3().y()*pow(t, 3.) + val.y());
        v2YInc += (1. - t)*pow(t, 2.)*(-seg.p0().y()*pow(1. - t, 3.) - 3.*seg.c1().y()*pow(1. - t, 2.)*t -
                 seg.p3().y()*pow(t, 3.) + val.y());
        v1Dec = (1. - t)*pow(t, 2.)*(3.*t*pow(1. - t, 2.));
        v2Dec = (1. - t)*pow(t, 2.)*(3.*(1. - t)*pow(t, 2.));
    }
    return qCubicSegment2D(seg.p0(), QPointF(v1XInc/v1Dec, v1YInc/v1Dec),
                           QPointF(v2XInc/v2Dec, v2YInc/v2Dec), seg.p3());
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
            v1XInc += pow(1. - t, 2.)*t*(-seg.p0().x()*pow(1. - t, 3.) - 3.*seg.c2().x()*(1. - t)*pow(t, 2.) -
                     seg.p3().x()*pow(t, 3.) + val.x());
            v2XInc += (1. - t)*pow(t, 2.)*(-seg.p0().x()*pow(1. - t, 3.) - 3.*seg.c1().x()*pow(1. - t, 2.)*t -
                     seg.p3().x()*pow(t, 3.) + val.x());
            v1YInc += pow(1. - t, 2.)*t*(-seg.p0().y()*pow(1. - t, 3.) - 3.*seg.c2().y()*(1. - t)*pow(t, 2.) -
                     seg.p3().y()*pow(t, 3.) + val.y());
            v2YInc += (1. - t)*pow(t, 2.)*(-seg.p0().y()*pow(1. - t, 3.) - 3.*seg.c1().y()*pow(1. - t, 2.)*t -
                     seg.p3().y()*pow(t, 3.) + val.y());
            v1Dec += pow(1. - t, 2.)*t*(3.*t*pow(1. - t, 2.));
            v2Dec += (1. - t)*pow(t, 2.)*(3.*(1. - t)*pow(t, 2.));
        }
        v1 = QPointF(v1XInc/v1Dec, v1YInc/v1Dec);
        v2 = QPointF(v2XInc/v2Dec, v2YInc/v2Dec);
    }
    return {seg.p0(), v1, v2, seg.p3()};
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
    if(dist) *dist = minDist;
    return bestPos;
}

void gDrawCosmeticEllipse(QPainter *p,
                          const QPointF &absPos,
                          const qreal& rX, const qreal& rY) {
    const QTransform &transform = p->transform();
    p->drawEllipse(absPos,
                   rX/transform.m11(),
                   rY/transform.m22());
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

    const SkScalar nextDist = (currP - nextP).length()*0.5f;
    if(smoothLen < nextDist) {
        vectP.setLength(smoothLen);
    } else {
        vectP.setLength(nextDist);
    }

    c1 = currP + vectP;

    vectP.negate();
    const SkScalar lastDist = (currP - lastP).length()*0.5f;
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

    const SkScalar nextDist = (currP - nextP).length()*0.5f;
    vectP.setLength(nextDist);

    c2 = currP + vectP;

    vectP.negate();
    const SkScalar lastDist = (currP - lastP).length()*0.5f;
    vectP.setLength(lastDist);
    c1 = currP + vectP;
}

void gGetMaxSmoothAbsCtrlsForPtBetween(
        const QPointF &lastP,
        const QPointF &currP,
        const QPointF &nextP,
        QPointF &c1, QPointF &c2) {
    QPointF sLastP = lastP - currP;
    sLastP = scalePointToNewLen(sLastP, 1);
    QPointF sNextP = nextP - currP;
    sNextP = scalePointToNewLen(sNextP, 1);
    QPointF vectP = (sLastP + sNextP)*0.5;
    vectP = QPointF(vectP.y(), -vectP.x());

    if(QPointF::dotProduct(vectP, lastP - currP) > 0) vectP = -vectP;

    const qreal nextDist = pointToLen(currP - nextP)*0.5;
    vectP = scalePointToNewLen(vectP, nextDist);

    c2 = currP + vectP;

    vectP = -vectP;
    const qreal lastDist = pointToLen(currP - lastP)*0.5;
    vectP = scalePointToNewLen(vectP, lastDist);
    c1 = currP + vectP;
}

template <typename T1, typename T2>
void smoothyAbsCtrlsForPtBetween(const T1 &lastP, const T1 &currP,
                                 const T1 &nextP, T1 &c1, T1 &c2,
                                 T2 smoothness) {
    smoothness = CLAMP(smoothness, -1, 1);

    if(smoothness > 0) {
        T1 maxC1; T1 maxC2;
        gGetMaxSmoothAbsCtrlsForPtBetween(lastP, currP, nextP, maxC1, maxC2);
        c1 = currP + (c1 - currP)*(1 - smoothness) + (maxC1 - currP)*smoothness;
        c2 = currP + (c2 - currP)*(1 - smoothness) + (maxC2 - currP)*smoothness;
    } else {
        const T2 smoothnessDec = smoothness + 1;
        c1 = currP + (c1 - currP)*smoothnessDec;
        c2 = currP + (c2 - currP)*smoothnessDec;
    }
}

void gSmoothyAbsCtrlsForPtBetween(const QPointF &lastP,
                                  const QPointF &currP,
                                  const QPointF &nextP,
                                  QPointF &c1, QPointF &c2,
                                  qreal smoothness) {
    smoothyAbsCtrlsForPtBetween(lastP, currP, nextP, c1, c2, smoothness);
}

void gSmoothyAbsCtrlsForPtBetween(const SkPoint &lastP,
                                  const SkPoint &currP,
                                  const SkPoint &nextP,
                                  SkPoint &c1, SkPoint &c2,
                                  SkScalar smoothness) {
    smoothyAbsCtrlsForPtBetween(lastP, currP, nextP, c1, c2, smoothness);
}

SkPath gPathToPolyline(const SkPath& path) {
    SkPath result;
    QPointF lastMovePos;
    QPointF lastPos;
    SkPath::Iter iter(path, false);
    for(;;) {
        qCubicSegment2D seg;
        SkPoint pts[4];
        switch(iter.next(pts, true, true)) {
        case SkPath::kLine_Verb: {
            QPointF pt1 = toQPointF(pts[1]);
            result.lineTo(pts[1]);
            lastPos = pt1;
            continue;
        }
        case SkPath::kQuad_Verb: {
            QPointF pt2 = toQPointF(pts[2]);
            seg = qCubicSegment2D::fromQuad(lastPos, toQPointF(pts[1]), pt2);
            lastPos = pt2;
        } break;
        case SkPath::kConic_Verb: {
            QPointF pt2 = toQPointF(pts[2]);
            seg = qCubicSegment2D::fromConic(lastPos, toQPointF(pts[1]), pt2,
                                             toQreal(iter.conicWeight()));
            lastPos = pt2;
        } break;
        case SkPath::kCubic_Verb: {
            QPointF pt3 = toQPointF(pts[3]);
            seg = qCubicSegment2D(lastPos, toQPointF(pts[1]),
                                  toQPointF(pts[2]), pt3);
            lastPos = pt3;
        } break;
        case SkPath::kClose_Verb: {
            result.close();
            continue;
        }
        case SkPath::kMove_Verb: {
            result.moveTo(pts[0]);
            lastMovePos = toQPointF(pts[0]);
            lastPos = lastMovePos;
            continue;
        }
        case SkPath::kDone_Verb:
            return result;
        }
        if(!seg.isLine()) {
            for(qreal len = 10; len < seg.length(); len += 10) {
                result.lineTo(toSkPoint(seg.posAtLength(len)));
            }
        }
        result.lineTo(toSkPoint(seg.p3()));
    }
}

void gForEverySegmentInPath(
        const SkPath& path,
        const std::function<void(const qCubicSegment2D&)>& func) {
    QPointF lastMovePos;
    QPointF lastPos;
    SkPath::Iter iter(path, false);
    for(;;) {
        SkPoint pts[4];
        switch(iter.next(pts, true, true)) {
        case SkPath::kLine_Verb: {
            QPointF pt1 = toQPointF(pts[1]);
            func(qCubicSegment2D(lastPos, lastPos, pt1, pt1));
            lastPos = pt1;
        } break;
        case SkPath::kQuad_Verb: {
            QPointF pt2 = toQPointF(pts[2]);
            func(qCubicSegment2D::fromQuad(lastPos, toQPointF(pts[1]), pt2));
            lastPos = pt2;
        } break;
        case SkPath::kConic_Verb: {
            QPointF pt2 = toQPointF(pts[2]);
            func(qCubicSegment2D::fromConic(lastPos, toQPointF(pts[1]), pt2,
                                            toQreal(iter.conicWeight())));
            lastPos = pt2;
        } break;
        case SkPath::kCubic_Verb: {
            QPointF pt3 = toQPointF(pts[3]);
            func(qCubicSegment2D(lastPos, toQPointF(pts[1]),
                                 toQPointF(pts[2]), pt3));
            lastPos = pt3;
        } break;
        case SkPath::kClose_Verb: {
            if(!isZero2Dec(pointToLen(lastPos - lastMovePos))) {
                func({lastPos, lastPos, lastMovePos, lastMovePos});
                lastPos = lastMovePos;
            }
        } break;
        case SkPath::kMove_Verb: {
            lastMovePos = toQPointF(pts[0]);
            lastPos = lastMovePos;
        } break;
        case SkPath::kDone_Verb:
            return;
        }
    }
}

void gForEverySegmentInPath(
        const SkPath& path,
        const std::function<void(const SkPath&)>& func) {
    SkPoint lastMovePos;
    SkPoint lastPos;
    SkPath::Iter iter(path, false);
    for(;;) {
        SkPoint pts[4];
        switch(iter.next(pts, true, true)) {
        case SkPath::kLine_Verb: {
            const SkPoint pt1 = pts[1];
            SkPath seg;
            seg.moveTo(lastPos);
            seg.lineTo(pt1);
            func(seg);
            lastPos = pt1;
        } break;
        case SkPath::kQuad_Verb: {
            const SkPoint pt2 = pts[2];
            SkPath seg;
            seg.moveTo(lastPos);
            seg.quadTo(pts[1], pt2);
            func(seg);
            lastPos = pt2;
        } break;
        case SkPath::kConic_Verb: {
            const SkPoint pt2 = pts[2];
            SkPath seg;
            seg.moveTo(lastPos);
            seg.conicTo(pts[1], pt2, iter.conicWeight());
            func(seg);
            lastPos = pt2;
        } break;
        case SkPath::kCubic_Verb: {
            const SkPoint pt3 = pts[3];
            SkPath seg;
            seg.moveTo(lastPos);
            seg.cubicTo(pts[1], pts[2], pt3);
            func(seg);
            lastPos = pt3;
        } break;
        case SkPath::kClose_Verb: {
            if(!isZero2Dec(pointToLen(lastPos - lastMovePos))) {
                SkPath seg;
                seg.moveTo(lastPos);
                seg.lineTo(lastMovePos);
                func(seg);
            }
        } break;
        case SkPath::kMove_Verb: {
            lastMovePos = pts[0];
            lastPos = lastMovePos;
        } break;
        case SkPath::kDone_Verb:
            return;
        }
    }
}


static void Perterb(SkPoint * const p,
                    const SkVector& tangent,
                    const SkScalar& scale) {
    SkVector normal = tangent;
    SkPointPriv::RotateCCW(&normal);
    normal.setLength(scale);
    *p += normal;
}

float randFloat() {
    return static_cast<float>(gRandF(-1, 1));
}

bool gDisplaceFilterPath(SkPath* const dst,
                         const SkPath& src,
                         const SkScalar &maxDev,
                         const SkScalar &segLen,
                         const SkScalar &smoothness,
                         const uint32_t &seedAssist) {
    if(segLen < 0.01f) return false;
    dst->reset();
    SkPathMeasure meas(src, false);

    const SkScalar scale = maxDev;
    SkPoint p;
    SkVector v;

    uint32_t seedContourInc = 0;;
    if(smoothness < 0.001f) {
        do {
            qsrand(seedAssist + seedContourInc);
            seedContourInc += 100;
            SkScalar length = meas.getLength();
//            if(segLen * 2 > length) {
//                meas.getSegment(0, length, dst, true);  // to short for us to mangle
//                continue;
//            }
            const int nTot = SkScalarCeilToInt(length / segLen);
            int n = nTot;
            SkScalar distance = 0;
            const SkScalar remLen = segLen*nTot - length;
            SkPoint firstP;
            if(meas.isClosed()) {
                n--;
                distance += (length + segLen)*0.5f;
            }

            if(meas.getPosTan(distance, &p, &v)) {
                //Perterb(&p, v, randFloat() * scale);
                dst->moveTo(p);
                firstP = p;
            }
            while(--n >= 0) {
                distance += segLen;
                if(meas.getPosTan(distance, &p, &v)) {
                    if(n == 0) {
                        const SkScalar scaleT = 1 - remLen/segLen;
                        Perterb(&p, v, randFloat() * scale * scaleT);
                    } else {
                        Perterb(&p, v, randFloat() * scale);
                    }
                    dst->lineTo(p);
                }
                if(distance + segLen > length) break;
            }
            if(meas.isClosed()) {
                distance = distance + segLen - length;
                while(--n >= 0) {
                    if(meas.getPosTan(distance, &p, &v)) {
                        if(n == 0) {
                            const SkScalar scaleT = 1 - remLen/segLen;
                            Perterb(&p, v, randFloat() * scale * scaleT);
                        } else {
                            Perterb(&p, v, randFloat() * scale);
                        }
                        dst->lineTo(p);
                    }
                    distance += segLen;
                }
                dst->close();
            }
        } while(meas.nextContour());
    } else {
        SkPoint firstP;
        SkPoint secondP;
        SkPoint thirdP;
        SkPoint lastP;
        SkPoint nextP;
        SkPoint currP;
        SkPoint lastC1;
        SkPoint c1;
        SkPoint c2;

        do {
            qsrand(seedAssist + seedContourInc);
            seedContourInc += 100;
            const SkScalar length = meas.getLength();
//            if(segLen * 2 > length) {
//                meas.getSegment(0, length, dst, true);  // to short for us to mangle
//                continue;
//            }
            int nTot = SkScalarCeilToInt(length / segLen);
            int n = nTot;
            SkScalar distance = 0;
            const SkScalar remLen = segLen*nTot - length;
            const SkScalar smoothLen = smoothness * segLen * 0.5f;

            if(meas.isClosed()) {
                n--;
                distance += (length + segLen)*0.5f;
            }

            if(meas.getPosTan(distance, &firstP, &v)) {
                //Perterb(&firstP, v, randFloat() * scale);
                lastP = firstP;
            }

            if(meas.isClosed()) {
                distance += segLen;
                if(meas.getPosTan(distance, &currP, &v)) {
                    Perterb(&currP, v, randFloat() * scale);
                    n--;
                    secondP = currP;
                }
                distance += segLen;
                if(meas.getPosTan(distance, &nextP, &v)) {
                    Perterb(&nextP, v, randFloat() * scale);
                    n--;
                    thirdP = nextP;

                    gGetSmoothAbsCtrlsForPtBetween(lastP, currP, nextP, c1, c2, smoothLen);

                    lastC1 = c1;

                    lastP = currP;
                    currP = nextP;
                }
            } else {
                currP = lastP;
                lastC1 = currP;
            }
            dst->moveTo(lastP);
            while(--n >= 0) {
                distance += segLen;
                if(meas.getPosTan(distance, &nextP, &v)) {
                    if(n == 0) {
                        const SkScalar scaleT = 1 - remLen/segLen;
                        Perterb(&nextP, v, randFloat() * scale * scaleT);

                    } else {
                        Perterb(&nextP, v, randFloat() * scale);
                    }
                    gGetSmoothAbsCtrlsForPtBetween(lastP, currP, nextP, c1, c2, smoothLen);


                    dst->cubicTo(lastC1, c2, currP);
                    lastC1 = c1;

                    lastP = currP;
                    currP = nextP;
                }
                if(distance + segLen > length) break;
            }

//            nextP = firstP;
//            getC1AndC2(lastP, currP, nextP,
//                       &c1, &c2, smoothLen);
//            dst->cubicTo(lastC1, c2, currP);
            if(meas.isClosed()) {
                distance = distance + segLen - length;
                while(--n >= 0) {
                    if(meas.getPosTan(distance, &nextP, &v)) {
                        if(n == 0) {
                            const SkScalar scaleT = 1 - remLen/segLen;
                            Perterb(&nextP, v, randFloat() * scale * scaleT);
                        } else {
                            Perterb(&nextP, v, randFloat() * scale);
                        }
                        gGetSmoothAbsCtrlsForPtBetween(lastP, currP, nextP, c1, c2, smoothLen);


                        dst->cubicTo(lastC1, c2, currP);
                        lastC1 = c1;

                        lastP = currP;
                        currP = nextP;
                    }
                    distance += segLen;
                }
                lastC1 = c1;

//                lastP = currP;
//                currP = nextP;
                nextP = firstP;

                gGetSmoothAbsCtrlsForPtBetween(lastP, currP, nextP, c1, c2, smoothLen);

                dst->cubicTo(lastC1, c2, currP);
                lastC1 = c1;

                lastP = currP;
                currP = nextP;
                nextP = secondP;
                gGetSmoothAbsCtrlsForPtBetween(lastP, currP, nextP, c1, c2, smoothLen);
                dst->cubicTo(lastC1, c2, currP);

                dst->close();
            }
        } while(meas.nextContour());
    }
    return true;
}

qreal gMapTToFragment(const qreal &minAbsT,
                      const qreal &maxAbsT,
                      const qreal &absT) {
    const qreal tFrag = maxAbsT - minAbsT;
    if(isZero6Dec(tFrag)) {
        if(isZero4Dec(absT - minAbsT)) return 0.5;
        RuntimeThrow("Cannot map to zero range");
    }
    if(tFrag < 0) RuntimeThrow("Cannot map to negative range");
    return (absT - minAbsT + (1 - maxAbsT))/tFrag;
}

qreal gMapTFromFragment(const qreal &minAbsT,
                        const qreal &maxAbsT,
                        const qreal &relT) {
    const qreal tFrag = maxAbsT - minAbsT;
    if(isZero6Dec(tFrag)) return minAbsT;
    if(tFrag < 0) RuntimeThrow("Cannot map to negative range");
    return minAbsT + relT*tFrag + maxAbsT - 1;
}

void gGetValuesForNodeInsertion(
        const QPointF &prevP1, QPointF &prevC2,
        QPointF &newC0, QPointF &newP1, QPointF &newC2,
        QPointF &nextC0, const QPointF &nextP1, qreal t) {
    t = CLAMP(t, 0, 1);
    const qreal oneMinusT = 1 - t;
    const QPointF P1_2 = prevC2*oneMinusT + nextC0*t;

    const QPointF newPrevC2 = prevP1*oneMinusT + prevC2*t;
    const QPointF newNextC0 = nextC0*oneMinusT + nextP1*t;

    newC0 = newPrevC2*oneMinusT + P1_2*t;
    newC2 = P1_2*oneMinusT + newNextC0*t;
    newP1 = newC0*oneMinusT + newC2*t;

    prevC2 = newPrevC2;
    nextC0 = newNextC0;
}

void gGetValuesForNodeRemoval(
        const QPointF &prevP1, QPointF &prevC2,
        const QPointF &pC0, const QPointF &pP1, const QPointF &pC2,
        QPointF &nextC0, const QPointF &nextP1,
        qreal t) {
    Q_UNUSED(pP1);
    t = CLAMP(t, 0, 1);
    if(isZero6Dec(t)) {
        prevC2 = pC2;
        return;
    }
    if(isOne6Dec(t)) {
        nextC0 = pC0;
        return;
    }
    const qreal tMinus1 = t - 1;
    prevC2 = (prevC2 + prevP1*tMinus1)/t;
    nextC0 = (nextP1*t - nextC0)/tMinus1;
}

QList<SkPath> gBreakApart(const SkPath &src) {
    QList<SkPath> result;

    SkPath::Iter iter(src, false);
    SkPath current;
    for(;;) {
        SkPoint pts[4];
        switch(iter.next(pts, true, true)) {
        case SkPath::kLine_Verb: {
            current.lineTo(pts[1]);
        } break;
        case SkPath::kQuad_Verb: {
            current.quadTo(pts[1], pts[2]);
        } break;
        case SkPath::kConic_Verb: {
            current.conicTo(pts[1], pts[2], iter.conicWeight());
        } break;
        case SkPath::kCubic_Verb: {
            current.cubicTo(pts[1], pts[2], pts[3]);
        } break;
        case SkPath::kClose_Verb: {
            current.close();
        } break;
        case SkPath::kMove_Verb: {
            if(!current.isEmpty()) {
                result << current;
                current.reset();
            }
            current.moveTo(pts[0]);
        } break;
        case SkPath::kDone_Verb: {
            if(!current.isEmpty()) {
                result << current;
                current.reset();
            }
            return result;
        }
        }
    }
}
