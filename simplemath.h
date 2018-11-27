#ifndef SIMPLEMATH_H
#define SIMPLEMATH_H
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define ROUND(x) ((int) ((x) + 0.5))
#define SIGN(x) ((x)>0?1:(-1))
#define SQR(x) ((x)*(x))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define MAX3(a, b, c) ((a)>(b)?MAX((a),(c)):MAX((b),(c)))
#define MIN3(a, b, c) ((a)<(b)?MIN((a),(c)):MIN((b),(c)))

#define PIf 3.14159265f
#define PI 3.14159265358979323846
#define RadToF 0.5/PI
#define RadToDeg 180/PI
#define sqrt_3 1.73205080757
#define sqrt_2 1.41421356237
#define sqrt_3f 1.73205080757f
#define sqrt_2f 1.41421356237f
#define v2to1div3 1.25992104989
#define sqrt3     1.73205080757
#define v2to2div3 1.58740105197
#include <QtCore>
#include "skqtconversions.h"

extern unsigned char truncateU8(const int &val);
extern int clampInt(int val, int min, int max);

extern bool isNonZero(const float &val);

extern bool isZero4Dec(const float val);
extern bool isZero4Dec(const double &val);

extern bool isZero2Dec(qreal val);

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

extern qreal clamp(qreal val, qreal min, qreal max);

extern qreal qclamp(qreal val, qreal min, qreal max);

extern qreal qRandF(const qreal &fMin = 0., const qreal &fMax = 1.);

extern QPointF symmetricToPos(QPointF toMirror,
                              QPointF mirrorCenter);
extern QPointF symmetricToPosNewLen(QPointF toMirror,
                                    QPointF mirrorCenter,
                                    qreal newLen);
extern qreal pointToLen(QPointF point);
extern SkScalar pointToLen(SkPoint point);
extern QPointF scalePointToNewLen(QPointF point, qreal newLen);

extern bool isPointZero(QPointF pos);
#endif // SIMPLEMATH_H
