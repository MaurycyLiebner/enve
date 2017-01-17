#include "pointhelpers.h"

qreal qRandF(qreal fMin, qreal fMax)
{
    qreal f = (qreal)qrand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

QPointF symmetricToPos(QPointF toMirror, QPointF mirrorCenter) {
    QPointF posDist = toMirror - mirrorCenter;
    return mirrorCenter - posDist;
}

QPointF symmetricToPosNewLen(QPointF toMirror, QPointF mirrorCenter, qreal newLen)
{
    QPointF posDist = toMirror - mirrorCenter;
    return mirrorCenter - scalePointToNewLen(posDist, newLen);
}

QPointF calcCubicBezierVal(QPointF p0, QPointF p1,
                           QPointF p2, QPointF p3,
                           qreal t) {
    return QPointF(calcCubicBezierVal(p0.x(), p1.x(), p2.x(), p3.x(), t),
                   calcCubicBezierVal(p0.y(), p1.y(), p2.y(), p3.y(), t));
}

qreal calcCubicBezierVal(qreal p0, qreal p1, qreal p2, qreal p3, qreal t) {
    return pow(1 - t, 3)*p0 +
            3*pow(1 - t, 2)*t*p1 +
            3*(1 - t)*t*t*p2 +
            t*t*t*p3;
}

// only for beziers that do not have multiple points of the same x value
// for qrealanimators
qreal tFromX(qreal p0x, qreal p1x, qreal p2x, qreal p3x, qreal x) {
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

qreal pointToLen(QPointF point) {
    return sqrt(point.x()*point.x() + point.y()*point.y());
}

bool isZero(qreal val) {
    return qAbs(val) < 0.0001;
}

bool isZero2Dec(qreal val) {
    return qAbs(val) < 0.01;
}

bool isZero1Dec(qreal val) {
    return qAbs(val) < 0.1;
}

bool isPointZero(QPointF pos) {
    return pointToLen(pos) < 0.0001;
}

QPointF scalePointToNewLen(QPointF point, qreal newLen) {
    if(isPointZero(point)) {
        return point;
    }
    return point * newLen / pointToLen(point);
}

QString boolToSql(bool bT) {
    return (bT) ? "1" : "0";
}

void getCtrlsSymmetricPos(QPointF endPos, QPointF startPos, QPointF centerPos,
                          QPointF *newEndPos, QPointF *newStartPos) {
    endPos = symmetricToPos(endPos, centerPos);
    qreal len1 = pointToLen(endPos);
    qreal len2 = pointToLen(startPos);
    qreal lenSum = len1 + len2;
    *newStartPos = (endPos*len1 + startPos*len2)/lenSum;
    *newEndPos = symmetricToPos(*newStartPos, centerPos);
}

void getCtrlsSmoothPos(QPointF endPos, QPointF startPos, QPointF centerPos,
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
            abs(QPointF::dotProduct(point2Rel, newStartDirection));
    *newStartPos = newStartDirection*startCtrlPtLen +
            centerPos;
    qreal endCtrlPtLen =
            abs(QPointF::dotProduct(point1Rel, newStartDirection));
    *newEndPos = -newStartDirection*endCtrlPtLen +
            centerPos;
}

qreal clamp(qreal val, qreal min, qreal max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

qreal qclamp(qreal val, qreal min, qreal max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}
#include <complex>

#define v2to1div3 1.25992104989
#define sqrt3     1.73205080757
#define v2to2div3 1.58740105197
#include <QList>
void getTValuesforBezier1D(const qreal &x0n,
                              const qreal &x1n,
                              const qreal &x2n,
                              const qreal &x3n,
                              const qreal &xn,
                              QList<std::complex<double> > *list) {
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
        list->append(t1);
    }
    if(t2.real() > 0. && t2.real() < 1.) {
        list->append(t2);
    }
    if(t3.real() > 0. && t3.real() < 1.) {
        list->append(t3);
    }
}

qreal get1DAccuracyValue(const qreal &x0,
                         const qreal &x1,
                         const qreal &x2,
                         const qreal &x3) {
    return qMax4(x0, x1, x2, x3) - qMin4(x0, x1, x2, x3);
}

#include <QDebug>
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
                         qreal *error) {
    QList<std::complex<double> > xValues;
    QList<std::complex<double> > yValues;


    getTValuesforBezier1D(x0, x1, x2, x3, x, &xValues);
    getTValuesforBezier1D(y0, y1, y2, y3, y, &yValues);
    qreal bestT = 0.;
    qreal minErrorT = 1000000.;
    foreach(const std::complex<double> &yVal, yValues) {
        qreal errorT = pointToLen(QPointF(calcCubicBezierVal(x0, x1, x2, x3,
                                                             yVal.real()),
                                          calcCubicBezierVal(y0, y1, y2, y3,
                                                             yVal.real())) -
                                 QPointF(x, y));
        if(errorT < minErrorT) {
            minErrorT = errorT;
            bestT = yVal.real();
        }
    }
    foreach(const std::complex<double> &xVal, xValues) {
        qreal errorT = pointToLen(QPointF(calcCubicBezierVal(x0, x1, x2, x3,
                                                             xVal.real()),
                                          calcCubicBezierVal(y0, y1, y2, y3,
                                                             xVal.real())) -
                                 QPointF(x, y));
        if(errorT < minErrorT) {
            minErrorT = errorT;
            bestT = xVal.real();
        }
    }

    if(error != NULL) *error = minErrorT;

    return bestT;
}

qreal getTforBezierPoint(const QPointF &p0,
                         const QPointF &p1,
                         const QPointF &p2,
                         const QPointF &p3,
                         const QPointF &p,
                         qreal *error) {
    return getTforBezierPoint(p0.x(), p1.x(), p2.x(), p3.x(), p.x(),
                              p0.y(), p1.y(), p2.y(), p3.y(), p.y(),
                              error);
}

qreal qMin4(qreal v1, qreal v2, qreal v3, qreal v4) {
    return qMin(v1, qMin(v2, qMin(v3, v4) ) );
}

qreal qMax4(qreal v1, qreal v2, qreal v3, qreal v4) {
    return qMax(v1, qMax(v2, qMax(v3, v4) ) );
}

QRectF qRectF4Points(QPointF p1, QPointF c1, QPointF c2, QPointF p2) {
    return QRectF(QPointF(qMin4(p1.x(), c1.x(), c2.x(), p2.x()),
                          qMin4(p1.y(), c1.y(), c2.y(), p2.y())),

                  QPointF(qMax4(p1.x(), c1.x(), c2.x(), p2.x()),
                          qMax4(p1.y(), c1.y(), c2.y(), p2.y())));
}
