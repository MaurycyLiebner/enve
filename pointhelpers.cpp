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
struct TValues {

    const std::complex<double> &getValue(int i) {
        if(i == 0) {
            return t1;
        } else if(i == 1) {
            return t2;
        } else {
            return t3;
        }
    }

    std::complex<double> t1;
    std::complex<double> t2;
    std::complex<double> t3;
};

#define v2to1div3 1.25992104989
#define sqrt3     1.73205080757
#define v2to2div3 1.58740105197

TValues getTValuesforBezier1D(const qreal &x0n,
                              const qreal &x1n,
                              const qreal &x2n,
                              const qreal &x3n,
                              const qreal &xn) {
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
    TValues values;
    values.t1 = (-var6 + (v2to1div3*var4)/var3 + v2to2div3*var3*0.5) / var5;
    values.t2 = (-4.*var6 - (2.*i*v2to1div3*(-i + sqrt3)*var4)/var3 +
                 i*v2to2div3*(i + sqrt3)*var3) / (4.*var5);
    values.t3 = (-4.*var6 + (2.*i*v2to1div3*(i + sqrt3)*var4)/var3 -
                 v2to2div3*(1. + i*sqrt3)*var3 ) / (4.*var5);

    return values;
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
                         qreal *error) {
    TValues xValues = getTValuesforBezier1D(x0, x1, x2, x3, x);
    TValues yValues = getTValuesforBezier1D(y0, y1, y2, y3, y);
    double minValDet = 4.;
    qreal bestT = 0.;
    for(int i = 0; i < 3; i++) {
        const std::complex<double> xVal = xValues.getValue(i);
        for(int j = 0; j < 3; j++) {
            const std::complex<double> yVal = yValues.getValue(j);
            double valDet = (1 + qAbs(xVal.imag()) + qAbs(yVal.imag()))*
                            (1 + qAbs(xVal.real() - yVal.real()));
            if(valDet < minValDet) {
                minValDet = valDet;
                bestT = (xVal.real() + yVal.real())*0.5;
            }
        }
    }
    *error = pointToLen(QPointF(calcCubicBezierVal(x0, x1, x2, x3, bestT),
                     calcCubicBezierVal(y0, y1, y2, y3, bestT)) -
            QPointF(x, y));
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
