#ifndef FITCURVES_H
#define FITCURVES_H

#include <functional>
#include <QtCore>

typedef struct Point2Struct {	/* 2d point */
    double x, y;
    } Point2;
typedef Point2 Vector2;
typedef Point2 *BezierCurve;
typedef std::function<void(const int n, const BezierCurve curve)> BezierHandler;

namespace FitCurves {
    extern void FitCurve(QVector<QPointF>& data, const double error,
                         const BezierHandler& bezierHandler);
    extern void FitCurve(Point2* const d, const int nPts, const double error,
                         const BezierHandler& bezierHandler);

};

#endif // FITCURVES_H
