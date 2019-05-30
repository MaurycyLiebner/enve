#ifndef SIMPLEMATH_H
#define SIMPLEMATH_H
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define ROUND(x) ((int)(x + 0.5))
#define SIGN(x) (x > 0 ? 1 : -1)
#define SQR(x) (x*x)
#define CLAMP(x, low, high)  (x > high ? high : (x < low ? low : x))
#define CLAMP01(x)  (x > 1 ? 1 : (x < 0 ? 0 : x))
#define MAX3(a, b, c) (a > b ? MAX(a,c) : MAX(b, c))
#define MIN3(a, b, c) (a < b ? MIN(a,c) : MIN(b, c))

#define PIf 3.14159265f
#define PI 3.14159265358979323846
#define RadToF (0.5/PI)
#define RadToDeg (180/PI)
#define sqrt_3 1.73205080757
#define sqrt_2 1.41421356237
#define sqrt_3f 1.73205080757f
#define sqrt_2f 1.41421356237f
#define v2to1div3 1.25992104989
#define sqrt3     1.73205080757
#define v2to2div3 1.58740105197

#define pow2(val) (val*val)
#define pow3(val) (val*val*val)

#include <QtCore>
#include "skia/skqtconversions.h"

extern qreal signedSquare(const qreal val);

extern qreal distSign(const QPointF& distPt);

extern unsigned char truncateU8(const int &val);
extern int clampInt(int val, int min, int max);

extern bool isNonZero(const float &val);

extern bool isZero4Dec(const float val);
extern bool isZero4Dec(const double &val);

extern double floor4Dec(const double& val);

extern bool isInteger4Dec(const double& val);

extern bool isZero6Dec(const double &val);
extern bool isZeroOrOne6Dec(const double &val);

extern bool isOne4Dec(const double &val);
extern bool isOne6Dec(const double &val);

extern bool isZero6Dec(const float &val);
extern bool isOne4Dec(const float &val);
extern bool isOne6Dec(const float &val);

extern bool isZero2Dec(const qreal &val);
extern bool isZero2Dec(const SkScalar &val);

extern bool isZero1Dec(qreal val);

extern qreal qMin4(qreal v1, qreal v2,
                   qreal v3, qreal v4);
extern qreal qMax4(qreal v1, qreal v2,
                   qreal v3, qreal v4);

extern QRectF qRectF4Points(QPointF p1, QPointF c1,
                            QPointF c2, QPointF p2);

extern bool isNonZero(const double &val);
extern void rotate(float rad_t, float *x_t, float *y_t);
extern void rotate(qreal rad_t, qreal *x_t, qreal *y_t);

extern void normalize(qreal *x_t, qreal *y_t, qreal dest_len = 1.);
extern float clamp(float val, float min_t, float max_t);
extern double getAngleF(double x1, double y1, double x2, double y2);
extern double getAngleDeg(double x1, double y1, double x2, double y2);

extern QPointF rotateVector90Degrees(const QPointF &pt);

extern qreal degreesBetweenVectors(const QPointF &pt1,
                                   const QPointF &pt2);

extern qreal radiansBetweenVectors(const QPointF &pt1,
                                   const QPointF &pt2);

template <typename T1, typename T2>
qreal clamp(const qreal &val, const T1 &min, const T2 &max) {
    static_assert((std::is_same<T1, qreal>::value ||
                  std::is_same<T1, int>::value) &&
                  (std::is_same<T2, qreal>::value ||
                  std::is_same<T2, int>::value),
                  "qreal can be clamped only with ints and qreals");
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

template <typename T1, typename T2>
float clamp(const float &val, const T1 &min, const T2 &max) {
    static_assert((std::is_same<T1, float>::value ||
                  std::is_same<T1, int>::value) &&
                  (std::is_same<T2, float>::value ||
                  std::is_same<T2, int>::value),
                  "float can be clamped only with ints and floats");
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

extern int clamp(const int& val, const int& min, const int& max);

extern qreal gRandF(const qreal &fMin = 0, const qreal &fMax = 1);
extern SkScalar gSkRandF(const SkScalar &fMin = 0, const SkScalar &fMax = 1);
extern QPointF symmetricToPos(const QPointF &toMirror,
                              const QPointF &mirrorCenter);
extern QPointF symmetricToPosNewLen(const QPointF& toMirror,
                                    const QPointF& mirrorCenter,
                                    const qreal& newLen);
extern qreal pointToLen(QPointF point);
extern SkScalar pointToLen(SkPoint point);
extern QPointF scalePointToNewLen(const QPointF &point,
                                  const qreal &newLen);
extern QPointF gRotPt(const QPointF& pt, const qreal& deg);
extern QPointF gQPointFDisplace(const QPointF& pt, const qreal &displ);

extern bool isPointZero(QPointF pos);
#endif // SIMPLEMATH_H
