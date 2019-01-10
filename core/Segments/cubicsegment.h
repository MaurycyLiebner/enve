#ifndef CUBICSEGMENT_H
#define CUBICSEGMENT_H
#include <skia/skiaincludes.h>
#include <QPointF>

template <typename T>
struct CubicSegment {
    T fP0;
    T fP1;
    T fP2;
    T fP3;

    //T closestPointTo(const T& point);

    template <typename T2> explicit operator CubicSegment<T2> () const {
        T2 nP0; switchSkQ(fP0, nP0);
        T2 nP1; switchSkQ(fP1, nP1);
        T2 nP2; switchSkQ(fP2, nP2);
        T2 nP3; switchSkQ(fP3, nP3);
        return { nP0, nP1, nP2, nP3 };
    }
};

typedef CubicSegment<qreal> qCubicSegment1D;
typedef CubicSegment<SkScalar> SkCubicSegment1D;
typedef CubicSegment<QPointF> qCubicSegment2D;
typedef CubicSegment<SkPoint> SkCubicSegment2D;

#endif // CUBICSEGMENT_H
