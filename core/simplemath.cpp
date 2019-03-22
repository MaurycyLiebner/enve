#include "simplemath.h"

qreal gRandF(const qreal &fMin, const qreal &fMax) {
    qreal f = static_cast<qreal>(qrand())/RAND_MAX;
    return fMin + f * (fMax - fMin);
}

SkScalar gSkRandF(const SkScalar &fMin, const SkScalar &fMax) {
    SkScalar f = static_cast<SkScalar>(qrand())/RAND_MAX;
    return fMin + f * (fMax - fMin);
}

qreal pointToLen(QPointF point) {
    return qSqrt(point.x()*point.x() + point.y()*point.y());
}

SkScalar pointToLen(SkPoint point) {
    return SkScalarSqrt(point.x()*point.x() + point.y()*point.y());
}

QPointF scalePointToNewLen(const QPointF& point,
                           const qreal& newLen) {
    if(isPointZero(point)) return point;
    return point * newLen / pointToLen(point);
}

qreal clamp(const qreal& val, const qreal& min, const qreal& max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

int clamp(const int& val, const int& min, const int& max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

bool isZero4Dec(const float val) {
    return abs(val) < 0.0001f;
}

bool isZero4Dec(const double& val) {
    return qAbs(val) < 0.0001;
}

bool isInteger4Dec(const double& val) {
    return isZero4Dec(val - qRound(val));
}

double floor4Dec(const double& val) {
    const double roundVal = qRound(val);
    if(isZero4Dec(val - roundVal)) return roundVal;
    return qFloor(val);
}

bool isZero6Dec(const double &val) {
    return qAbs(val) < 0.000001;
}

bool isOne4Dec(const double &val) {
    return isZero4Dec(val - 1);
}

bool isOne6Dec(const double &val) {
    return isZero6Dec(val - 1);
}

bool isZero6Dec(const float &val) {
    return qAbs(val) < 0.000001f;
}

bool isOne4Dec(const float &val) {
    return isZero4Dec(val - 1);
}

bool isOne6Dec(const float &val) {
    return isZero6Dec(val - 1);
}

bool isZeroOrOne6Dec(const double &val) {
    if(isZero6Dec(val)) return true;
    if(isOne6Dec(val)) return true;
    return false;
}

bool isZero2Dec(const qreal& val) {
    return qAbs(val) < 0.01;
}

bool isZero2Dec(const SkScalar &val) {
    return qAbs(val) < 0.01f;
}

bool isZero1Dec(qreal val) {
    return qAbs(val) < 0.1;
}

bool isPointZero(QPointF pos) {
    return pointToLen(pos) < 0.0001;
}

bool isNonZero(const float &val) {
    return val > 0.0001f || val < - 0.0001f;
}

bool isNonZero(const double &val) {
    return val > 0.0001 || val < - 0.0001;
}

void rotate(float rad_t, float *x_t, float *y_t) {
    float cos_hue = cosf(rad_t);
    float sin_hue = sinf(rad_t);
    float x_rotated_t = *x_t*cos_hue - *y_t*sin_hue;
    float y_rotated_t = *x_t*sin_hue + *y_t*cos_hue;
    *x_t = x_rotated_t;
    *y_t = y_rotated_t;
}

void rotate(qreal rad_t, qreal *x_t, qreal *y_t) {
    qreal cos_hue = cos(rad_t);
    qreal sin_hue = sin(rad_t);
    qreal x_rotated_t = *x_t*cos_hue - *y_t*sin_hue;
    qreal y_rotated_t = *x_t*sin_hue + *y_t*cos_hue;
    *x_t = x_rotated_t;
    *y_t = y_rotated_t;
}

unsigned char truncateU8(const int &val) {
    if(val > 255) {
        return 255;
    } else if(val < 0) {
        return 0;
    }
    return static_cast<unsigned char>(val);
}

int clampInt(int val, int min, int max) {
    if(val > max) {
        return max;
    } else if(val < min) {
        return min;
    } else {
        return val;
    }
}

float clamp(float val, float min_t, float max_t) {
    if(val > max_t)
    {
        return max_t;
    }
    if(val < min_t)
    {
        return min_t;
    }
    return val;
}

double getAngleF(double x1, double y1, double x2, double y2) {
    double dot = x1*x2 + y1*y2;
    double det = x1*y2 - y1*x2;
    return atan2(det, dot)*RadToF + 0.5;
}

double getAngleDeg(double x1, double y1, double x2, double y2)
{
    double dot = x1*x2 + y1*y2;
    double det = x1*y2 - y1*x2;
    return atan2(det, dot)*RadToDeg + 180;
}

void normalize(qreal *x_t, qreal *y_t, qreal dest_len) {
    qreal x_val = *x_t;
    qreal y_val = *y_t;
    qreal curr_len = sqrt(x_val*x_val + y_val*y_val);
    *x_t = x_val*dest_len/curr_len;
    *y_t = y_val*dest_len/curr_len;
}



qreal qMin4(qreal v1, qreal v2, qreal v3, qreal v4) {
    return qMin(v1, qMin(v2, qMin(v3, v4) ) );
}

qreal qMax4(qreal v1, qreal v2, qreal v3, qreal v4) {
    return qMax(v1, qMax(v2, qMax(v3, v4) ) );
}

QRectF qRectF4Points(QPointF p1, QPointF c1,
                     QPointF c2, QPointF p2) {
    return QRectF(QPointF(qMin4(p1.x(), c1.x(), c2.x(), p2.x()),
                          qMin4(p1.y(), c1.y(), c2.y(), p2.y())),

                  QPointF(qMax4(p1.x(), c1.x(), c2.x(), p2.x()),
                          qMax4(p1.y(), c1.y(), c2.y(), p2.y())));
}

QPointF rotateVector90Degrees(const QPointF &pt) {
    return QPointF(-pt.y(), pt.x()); // y is downwards
}

qreal degreesBetweenVectors(const QPointF &pt1,
                            const QPointF &pt2) {
    return radiansBetweenVectors(pt1, pt2)*180./M_PI;
}

qreal radiansBetweenVectors(const QPointF &pt1,
                            const QPointF &pt2) {
    qreal dot = pt1.x()*pt2.x() + pt1.y()*pt2.y();
    qreal det = pt1.x()*pt2.y() - pt1.y()*pt2.x();
    return atan2(det, dot);
}

QPointF gRotPt(const QPointF &pt, const qreal &deg) {
    return {pt.x() * cos(deg*PI/180) - pt.y() * sin(deg*PI/180),
            pt.x() * sin(deg*PI/180) + pt.y() * cos(deg*PI/180)};
}

QPointF gQPointFDisplace(const QPointF& pt, const qreal &displ) {
    return QPointF(pt.x() + gRandF(-displ, displ),
                   pt.y() + gRandF(-displ, displ));
}
