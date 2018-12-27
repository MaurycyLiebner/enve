#include "pointhelpers.h"
#include <QtMath>
#include <complex>
#include <QDebug>
#include <QMatrix>
#include <QList>

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

QPointF calcCubicBezierVal(QPointF p0, QPointF p1,
                           QPointF p2, QPointF p3,
                           qreal t) {
    return QPointF(calcCubicBezierVal(p0.x(), p1.x(), p2.x(), p3.x(), t),
                   calcCubicBezierVal(p0.y(), p1.y(), p2.y(), p3.y(), t));
}

qreal calcCubicBezierVal(qreal p0, qreal p1,
                         qreal p2, qreal p3,
                         qreal t) {
    return qPow(1 - t, 3)*p0 +
            3*qPow(1 - t, 2)*t*p1 +
            3*(1 - t)*t*t*p2 +
            t*t*t*p3;
}

// only for beziers that do not have multiple points of the same x value
// for qrealanimators
qreal tFromX(qreal p0x, qreal p1x,
             qreal p2x, qreal p3x,
             qreal x) {
    qreal minT = 0.;
    qreal maxT = 1.;
    qreal xGuess;
    qreal guessT;
    do {
        guessT = (maxT + minT)*0.5;
        xGuess = calcCubicBezierVal(p0x, p1x, p2x, p3x, guessT);
        if(xGuess > x) {
            maxT = guessT;
        } else {
            minT = guessT;
        }
    } while(qAbs(xGuess - x) > 0.0001);
    return guessT;
}

void getCtrlsSymmetricPos(QPointF endPos, QPointF startPos,
                          QPointF centerPos,
                          QPointF *newEndPos, QPointF *newStartPos) {
    endPos = symmetricToPos(endPos, centerPos);
    qreal len1 = pointToLen(endPos);
    qreal len2 = pointToLen(startPos);
    qreal lenSum = len1 + len2;
    *newStartPos = (endPos*len1 + startPos*len2)/lenSum;
    *newEndPos = symmetricToPos(*newStartPos, centerPos);
}

void getCtrlsSmoothPos(QPointF endPos, QPointF startPos,
                       QPointF centerPos,
                       QPointF *newEndPos, QPointF *newStartPos) {
    QPointF symEndPos = symmetricToPos(endPos, centerPos);
    qreal len1 = pointToLen(symEndPos);
    qreal len2 = pointToLen(startPos);
    qreal lenSum = len1 + len2;
    QPointF point1Rel = endPos - centerPos;
    QPointF point2Rel = startPos - centerPos;
    QPointF newStartDirection =
            scalePointToNewLen(
                (symEndPos*len1 + startPos*len2)/lenSum - centerPos,
                1.);
    qreal startCtrlPtLen =
            qAbs(QPointF::dotProduct(point2Rel, newStartDirection));
    *newStartPos = newStartDirection*startCtrlPtLen +
            centerPos;
    qreal endCtrlPtLen =
            qAbs(QPointF::dotProduct(point1Rel, newStartDirection));
    *newEndPos = -newStartDirection*endCtrlPtLen +
            centerPos;
}

void getClosestTValuesBezier1D(const qreal &v0n,
                               const qreal &v1n,
                               const qreal &v2n,
                               const qreal &v3n,
                               const qreal &vn,
                               QList<qreal> *list) {
    std::complex<double> v0 = std::complex<double>(v0n, 0.);
    std::complex<double> v1 = std::complex<double>(v1n, 0.);
    std::complex<double> v2 = std::complex<double>(v2n, 0.);
    std::complex<double> v3 = std::complex<double>(v3n, 0.);
    std::complex<double> v = std::complex<double>(vn, 0.);
    std::complex<double> var1 = sqrt(pow(v1, 2.) + pow(v2, 2.) - v1*(v2 + v3) -
                                 v0*(v3 - v2));
    std::complex<double> var2 = v0 - 3.*v1 + 3.*v2 - v3;
    std::complex<double> var3 = v0 - 2.*v1 + v2;
    std::complex<double> sol1 = (var1 - var3)/-var2;
    std::complex<double> sol2 = (var1 + var3)/var2;

    std::complex<double> var4 =
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

    std::complex<double> sol3 = (-2.*(v0 - 2.*v1 + v2) + (2.*v2to1div3*
                    (pow(v1,2.) + pow(v2,2.) + v0*(-v2 + v3) - v1*(v2 + v3)))/
                  var4 + v2to2div3*
                  var4)/(2.*(-v0 + 3.*v1 - 3.*v2 + v3));
    std::complex<double> sol4 =
            (-36.*(v0 - 2.*v1 + v2) - (std::complex<double>(0.,18.)*v2to1div3*
                     (std::complex<double>(0.,-1.) + sqrt(3.))*
                     (pow(v1,2.) + pow(v2,2.) + v0*(-v2 + v3) - v1*(v2 + v3)))/
                   var4 + std::complex<double>(0,9)*v2to2div3*
                   (std::complex<double>(0.,1.) + sqrt(3.))*
                   var4)/(36.*(-v0 + 3.*v1 - 3.*v2 + v3));

    std::complex<double> sol5 =
            (-36.*(v0 - 2.*v1 + v2) + (std::complex<double>(0.,18.)*v2to1div3*
           (std::complex<double>(0.,1.) + sqrt(3.))*
           (pow(v1,2.) + pow(v2,2.) + v0*(-v2 + v3) - v1*(v2 + v3)))/
         var4 - 9.*v2to2div3*
         (1. + std::complex<double>(0.,1.)*sqrt(3.))*var4)/
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

void getTValuesforBezier1D(const qreal &x0n,
                          const qreal &x1n,
                          const qreal &x2n,
                          const qreal &x3n,
                          const qreal &xn,
                          QList<qreal> *list) {
    std::complex<double> i = std::complex<double>(0., 1.);
    std::complex<double> x0 = std::complex<double>(x0n, 0.);
    std::complex<double> x1 = std::complex<double>(x1n, 0.);
    std::complex<double> x2 = std::complex<double>(x2n, 0.);
    std::complex<double> x3 = std::complex<double>(x3n, 0.);
    std::complex<double> x = std::complex<double>(xn, 0.);
    std::complex<double> x0sq = pow(x0, 2.);
    std::complex<double> x1sq = pow(x1, 2.);
    std::complex<double> x2sq = pow(x2, 2.);
    std::complex<double> x3sq = pow(x3, 2.);
    std::complex<double> xsq = pow(x, 2.);
    std::complex<double> x1th = pow(x1, 3.);
    std::complex<double> x2th = pow(x2, 3.);
    std::complex<double> _3x2x3 = 3.*x2*x3;
    std::complex<double> _x0x1 = x0*x1;
    std::complex<double> var1 = pow(x0 - 3.*x1 + 3.*x2 - x3, 2.);
    std::complex<double> var2 = sqrt(var1*(-3.*x1sq*x2sq + 4.*x0*x2th + xsq*var1 +
                                4.*x1th*x3 - 2.*_x0x1*_3x2x3 + x0sq*x3sq -
                                2.*x*(2.*x1th + 2.*x2th - 3.*x1sq*(x2 - 2.*x3) +
                                x0sq*x3 - 3.*x1*(x0 + x2)*(x2 + x3) +
                                x0*(6.*x2sq - _3x2x3 + x3sq))) );
    std::complex<double> var3 = pow(-2.*x1th + 3.*_x0x1*x2 + 3.*x1sq*x2 -
                                6.*x0*x2sq + 3.*x1*x2sq - 2.*x2th + x*var1 -
                                x0sq*x3 + 3.*_x0x1*x3 - 6.*x1sq*x3 + x0*_3x2x3 +
                                x1*_3x2x3 - x0*x3sq + var2, 1./3.);
    std::complex<double> var4 = x1sq + x2sq + x0*(-x2 + x3) - x1*(x2 + x3);
    std::complex<double> var5 = -x0 + 3.*x1 - 3.*x2 + x3;
    std::complex<double> var6 = x0 - 2.*x1 + x2;
    std::complex<double> t1 = (-var6 + (v2to1div3*var4)/var3 + v2to2div3*var3*0.5) / var5;
    std::complex<double> t2 = (-4.*var6 - (2.*i*v2to1div3*(-i + sqrt3)*var4)/var3 +
                 i*v2to2div3*(i + sqrt3)*var3) / (4.*var5);
    std::complex<double> t3 = (-4.*var6 + (2.*i*v2to1div3*(i + sqrt3)*var4)/var3 -
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

qreal getClosestTValueBezier2D(const QPointF &p0,
                               const QPointF &p1,
                               const QPointF &p2,
                               const QPointF &p3,
                               const QPointF &p,
                               QPointF *bestPosPtr,
                               qreal *errorPtr) {
    QList<qreal> xValues;
    QList<qreal> yValues;

    getClosestTValuesBezier1D(p0.x(), p1.x(), p2.x(), p3.x(), p.x(), &xValues);
    getClosestTValuesBezier1D(p0.y(), p1.y(), p2.y(), p3.y(), p.y(), &yValues);
    qreal bestT = 0.;
    QPointF bestPos;
    qreal minErrorT = 1000000.;
    Q_FOREACH(const qreal &yVal, yValues) {
        QPointF posT = QPointF(calcCubicBezierVal(p0.x(), p1.x(), p2.x(), p3.x(),
                                                  yVal),
                               calcCubicBezierVal(p0.y(), p1.y(), p2.y(), p3.y(),
                                                  yVal));
        qreal errorT = pointToLen(posT - QPointF(p.x(), p.y()));
        if(errorT < minErrorT) {
            bestPos = posT;
            minErrorT = errorT;
            bestT = yVal;
        }
    }
    Q_FOREACH(const qreal &xVal, xValues) {
        QPointF posT = QPointF(calcCubicBezierVal(p0.x(), p1.x(), p2.x(), p3.x(),
                                                  xVal),
                               calcCubicBezierVal(p0.y(), p1.y(), p2.y(), p3.y(),
                                                  xVal));
        qreal errorT = pointToLen(posT - QPointF(p.x(), p.y()));
        if(errorT < minErrorT) {
            bestPos = posT;
            minErrorT = errorT;
            bestT = xVal;
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

void bezierLeastSquareV1V2(const QPointF &v0,
                           QPointF &v1, QPointF &v2,
                           const QPointF &v3,
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
        v1XInc += pow(1. - t, 2.)*t*(-v0.x()*pow(1. - t, 3.) - 3.*v2.x()*(1. - t)*pow(t, 2.) -
                 v3.x()*pow(t, 3.) + val.x());
        v2XInc += (1. - t)*pow(t, 2.)*(-v0.x()*pow(1. - t, 3.) - 3.*v1.x()*pow(1. - t, 2.)*t -
                 v3.x()*pow(t, 3.) + val.x());
        v1YInc += pow(1. - t, 2.)*t*(-v0.y()*pow(1. - t, 3.) - 3.*v2.y()*(1. - t)*pow(t, 2.) -
                 v3.y()*pow(t, 3.) + val.y());
        v2YInc += (1. - t)*pow(t, 2.)*(-v0.y()*pow(1. - t, 3.) - 3.*v1.y()*pow(1. - t, 2.)*t -
                 v3.y()*pow(t, 3.) + val.y());
        v1Dec = (1. - t)*pow(t, 2.)*(3.*t*pow(1. - t, 2.));
        v2Dec = (1. - t)*pow(t, 2.)*(3.*(1. - t)*pow(t, 2.));
    }
    v1 = QPointF(v1XInc/v1Dec, v1YInc/v1Dec);
    v2 = QPointF(v2XInc/v2Dec, v2YInc/v2Dec);
}

void bezierLeastSquareV1V2(const QPointF &v0,
                           QPointF &v1, QPointF &v2,
                           const QPointF &v3,
                           const QList<QPointF> &vs,
                           const int &minVs,
                           const int &maxVs) {
    for(int j = 0; j < 50; j++) {
        qreal v1XInc = 0.;
        qreal v1Dec = 0.;
        qreal v2XInc = 0.;
        qreal v2Dec = 0.;
        qreal v1YInc = 0.;
        qreal v2YInc = 0.;
        for(int i = minVs; i <= maxVs; i++) {
            const QPointF &val = vs.at(i);
            qreal t = ((qreal)i - minVs)/(maxVs - minVs);
            //qreal t = getClosestTValueBezier2D(v0, v1, v2, v3, val);
            v1XInc += pow(1. - t, 2.)*t*(-v0.x()*pow(1. - t, 3.) - 3.*v2.x()*(1. - t)*pow(t, 2.) -
                     v3.x()*pow(t, 3.) + val.x());
            v2XInc += (1. - t)*pow(t, 2.)*(-v0.x()*pow(1. - t, 3.) - 3.*v1.x()*pow(1. - t, 2.)*t -
                     v3.x()*pow(t, 3.) + val.x());
            v1YInc += pow(1. - t, 2.)*t*(-v0.y()*pow(1. - t, 3.) - 3.*v2.y()*(1. - t)*pow(t, 2.) -
                     v3.y()*pow(t, 3.) + val.y());
            v2YInc += (1. - t)*pow(t, 2.)*(-v0.y()*pow(1. - t, 3.) - 3.*v1.y()*pow(1. - t, 2.)*t -
                     v3.y()*pow(t, 3.) + val.y());
            v1Dec += pow(1. - t, 2.)*t*(3.*t*pow(1. - t, 2.));
            v2Dec += (1. - t)*pow(t, 2.)*(3.*(1. - t)*pow(t, 2.));
        }
        v1 = QPointF(v1XInc/v1Dec, v1YInc/v1Dec);
        v2 = QPointF(v2XInc/v2Dec, v2YInc/v2Dec);
    }
}

qreal get1DAccuracyValue(const qreal &x0,
                         const qreal &x1,
                         const qreal &x2,
                         const qreal &x3) {
    return qMax4(x0, x1, x2, x3) - qMin4(x0, x1, x2, x3);
}

QPointF getClosestPointOnLineSegment(const QPointF &a,
                                     const QPointF &b,
                                     const QPointF &p) {
    QVector2D AP = QVector2D(p - a); //Vector from A to P
    QVector2D AB = QVector2D(b - a); //Vector from A to B

    qreal magnitudeAB = AB.lengthSquared(); //Magnitude of AB vector (it's length squared)
    qreal ABAPproduct = QVector2D::dotProduct(AP, AB); //The DOT product of a_to_p and a_to_b
    qreal distance = ABAPproduct / magnitudeAB; //The normalized "distance" from a to your closest point

    if(distance < 0.) { //Check if P projection is over vectorAB
        return a;
    } else if(distance > 1.) {
        return b;
    } else {
        return a + (AB * distance).toPointF();
    }
}

QPointF closestPointOnRect(const QRectF &rect,
                           const QPointF &point,
                           qreal *dist) {
    qreal minDist = DBL_MAX;
    QPointF bestPos;
    if(point.y() > rect.bottom()) {
        // check bottom
        QPointF pt = getClosestPointOnLineSegment(rect.bottomLeft(),
                                                  rect.bottomRight(),
                                                  point);
        qreal dist = pointToLen(pt - point);
        if(dist < minDist) {
            minDist = dist;
            bestPos = pt;
        }
    } else if(point.y() < rect.top()) {
        // check top
        QPointF pt = getClosestPointOnLineSegment(rect.topLeft(),
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
        QPointF pt = getClosestPointOnLineSegment(rect.topRight(),
                                                  rect.bottomRight(),
                                                  point);
        qreal dist = pointToLen(pt - point);
        if(dist < minDist) {
            minDist = dist;
            bestPos = pt;
        }
    } else if(point.y() < rect.left()) {
        // check left
        QPointF pt = getClosestPointOnLineSegment(rect.bottomLeft(),
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

qreal getTforBezierPoint(const qreal &x0,
                         const qreal &x1,
                         const qreal &x2,
                         const qreal &x3,
                         const qreal &x,
                         const qreal &y0,
                         const qreal &y1,
                         const qreal &y2,
                         const qreal &y3,
                         const qreal &y,
                         qreal *error,
                         QPointF *bestPosPtr,
                         const bool &fineTune) {
    QList<qreal> xValues;
    QList<qreal> yValues;


    getTValuesforBezier1D(x0, x1, x2, x3, x, &xValues);
    getTValuesforBezier1D(y0, y1, y2, y3, y, &yValues);
    qreal bestT = 0.;
    QPointF bestPos;
    qreal minErrorT = 1000000.;
    Q_FOREACH(const qreal &yVal, yValues) {
        QPointF posT = QPointF(calcCubicBezierVal(x0, x1, x2, x3,
                                                  yVal),
                               calcCubicBezierVal(y0, y1, y2, y3,
                                                  yVal));
        qreal errorT = pointToLen(posT - QPointF(x, y));
        if(errorT < minErrorT) {
            bestPos = posT;
            minErrorT = errorT;
            bestT = yVal;
        }
    }
    Q_FOREACH(const qreal &xVal, xValues) {
        QPointF posT = QPointF(calcCubicBezierVal(x0, x1, x2, x3,
                                                  xVal),
                               calcCubicBezierVal(y0, y1, y2, y3,
                                                  xVal));
        qreal errorT = pointToLen(posT - QPointF(x, y));
        if(errorT < minErrorT) {
            bestPos = posT;
            minErrorT = errorT;
            bestT = xVal;
        }
    }

    if(fineTune) {
        if(minErrorT < 999999.) { // check if found
            qreal incT = 0.1;
            qreal lastDisplacement = 100000.;
            while(lastDisplacement > 1.) {
                bool foundBetter = false;
                qreal nextT = clamp(bestT + incT, 0., 1.);
                QPointF nextPosT = QPointF(calcCubicBezierVal(x0, x1, x2, x3, nextT),
                                       calcCubicBezierVal(y0, y1, y2, y3, nextT));
                lastDisplacement = pointToLen(nextPosT - bestPos);
                qreal nextErrorT = pointToLen(nextPosT - QPointF(x, y));
                if(nextErrorT < minErrorT) {
                    bestPos = nextPosT;
                    minErrorT = nextErrorT;
                    bestT = nextT;
                    foundBetter = true;
                }

                qreal prevT = clamp(bestT - incT, 0., 1.);
                QPointF prevPosT = QPointF(calcCubicBezierVal(x0, x1, x2, x3, prevT),
                                       calcCubicBezierVal(y0, y1, y2, y3, prevT));
                lastDisplacement = qMax(lastDisplacement,
                                        pointToLen(prevPosT - bestPos));
                qreal prevErrorT = pointToLen(prevPosT - QPointF(x, y));
                if(prevErrorT < minErrorT) {
                    bestPos = prevPosT;
                    minErrorT = prevErrorT;
                    bestT = prevT;
                    foundBetter = true;
                }

                if(!foundBetter) incT *= 0.5;
            }
        }
    }

    if(error != nullptr) *error = minErrorT;
    if(bestPosPtr != nullptr) *bestPosPtr = bestPos;
    return bestT;
}


qreal getBezierTValueForX(const qreal &x0,
                         const qreal &x1,
                         const qreal &x2,
                         const qreal &x3,
                         const qreal &x,
                         qreal *error) {
    if(qAbs(x0 - x) < 0.01) return x0;
    if(qAbs(x3 - x) < 0.01) return x3;

    QList<qreal> xValues;

    getTValuesforBezier1D(x0, x1, x2, x3, x, &xValues);
    qreal bestT = 0.;
    qreal minErrorT = 1000000.;

    Q_FOREACH(const qreal &xVal, xValues) {
        qreal errorT = qAbs(calcCubicBezierVal(x0, x1, x2, x3,
                                          xVal) - x);
        if(errorT < minErrorT) {
            minErrorT = errorT;
            bestT = xVal;
        }
    }

    if(error != nullptr) *error = minErrorT;

    return bestT;
}

qreal getBezierTValueForXAssumeNoOverlapGrowingOnly(const qreal &x0,
                         const qreal &x1,
                         const qreal &x2,
                         const qreal &x3,
                         const qreal &x,
                         const qreal &minT,
                         const qreal &maxT,
                         const qreal &maxError,
                         qreal *error) {
    qreal tGuess = (maxT + minT)*0.5;
    qreal guessVal = calcCubicBezierVal(x0, x1, x2, x3, tGuess);
    qreal errorT = qAbs(guessVal - x);
    if(errorT < maxError) {
        if(error != nullptr) {
            *error = errorT;
        }
        return tGuess;
    }
    if(guessVal > x) {
        return getBezierTValueForXAssumeNoOverlapGrowingOnly(x0, x1, x2, x3, x,
                                           minT, tGuess, maxError, error);
    }
    return getBezierTValueForXAssumeNoOverlapGrowingOnly(x0, x1, x2, x3, x,
                                       tGuess, maxT, maxError, error);
}

qreal getBezierTValueForXAssumeNoOverlapGrowingOnly(const qreal &x0,
                         const qreal &x1,
                         const qreal &x2,
                         const qreal &x3,
                         const qreal &x,
                         const qreal &maxError,
                         qreal *error) {
    if(x0 > x3) {
        return getBezierTValueForXAssumeNoOverlapGrowingOnly(x3, x2, x1, x0, x,
                                                             maxError, error);
    }
    if(qAbs(x0 - x) < 0.01) return x0;
    if(qAbs(x3 - x) < 0.01) return x3;
    return getBezierTValueForXAssumeNoOverlapGrowingOnly(x0, x1, x2, x3,
                                                         x, 0., 1., maxError,
                                                         error);
}

qreal getTforBezierPoint(const QPointF &p0,
                         const QPointF &p1,
                         const QPointF &p2,
                         const QPointF &p3,
                         const QPointF &p,
                         qreal *error,
                         QPointF *bestPosPtr) {
    return getTforBezierPoint(p0.x(), p1.x(), p2.x(), p3.x(), p.x(),
                              p0.y(), p1.y(), p2.y(), p3.y(), p.y(),
                              error, bestPosPtr);
}

void drawCosmeticEllipse(QPainter *p,
                         const QPointF &absPos,
                         qreal rX, qreal rY) {
    const QTransform &transform = p->transform();
    p->drawEllipse(absPos,
                   rX/transform.m11(),
                   rY/transform.m22());
}


qreal distBetweenTwoPoints(QPointF point1, QPointF point2) {
    QPointF dPoint = point1 - point2;
    return sqrt(dPoint.x()*dPoint.x() + dPoint.y()*dPoint.y());
}


bool doesPathIntersectWithCircle(const QPainterPath &path,
                                 qreal xRadius, qreal yRadius,
                                 QPointF center) {
    QPainterPath circlePath;
    circlePath.addEllipse(center, xRadius, yRadius);
    return circlePath.intersects(path);
}

bool doesPathNotContainCircle(const QPainterPath &path,
                              qreal xRadius, qreal yRadius,
                              QPointF center) {
    QPainterPath circlePath;
    circlePath.addEllipse(center, xRadius, yRadius);
    return !path.contains(circlePath);
}

QPointF getCenterOfPathIntersectionWithCircle(const QPainterPath &path,
                                              qreal xRadius, qreal yRadius,
                                              QPointF center) {
    QPainterPath circlePath;
    circlePath.addEllipse(center, xRadius, yRadius);
    return circlePath.intersected(path).boundingRect().center();
}

QPointF getCenterOfPathDifferenceWithCircle(const QPainterPath &path,
                                            qreal xRadius, qreal yRadius,
                                            QPointF center) {
    QPainterPath circlePath;
    circlePath.addEllipse(center, xRadius, yRadius);
    return circlePath.subtracted(path).boundingRect().center();
}

QPointF getPointClosestOnPathTo(const QPainterPath &path,
                                QPointF relPos,
                                qreal xRadiusScaling,
                                qreal yRadiusScaling) {
    bool (*checkerFunc)(const QPainterPath &, qreal, qreal, QPointF);
    QPointF (*centerFunc)(const QPainterPath &, qreal, qreal, QPointF);
    if(path.contains(relPos)) {
        checkerFunc = &doesPathNotContainCircle;
        centerFunc = &getCenterOfPathDifferenceWithCircle;
    } else {
        checkerFunc = &doesPathIntersectWithCircle;
        centerFunc = &getCenterOfPathIntersectionWithCircle;
    }
    qreal radius = 1.;
    while(true) {
        if(checkerFunc(path,
                       xRadiusScaling*radius,
                       yRadiusScaling*radius,
                       relPos)) {
            return centerFunc(path,
                              xRadiusScaling*radius,
                              yRadiusScaling*radius,
                              relPos);
        }
        radius += 1.;
    }
}
