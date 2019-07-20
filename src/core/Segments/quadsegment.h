#ifndef QUADSEGMENT_H
#define QUADSEGMENT_H
#include <QPointF>
class SkPoint;
template <typename T>
struct CubicSegment;

template <typename T>
struct QuadSegment {
    T p0;
    T p1;
    T p2;

    T closestPointTo(const T& point);

    template <typename T2> explicit operator QuadSegment<T2> () const {
        T2 nP0; switchSkQ(p0, nP0);
        T2 nP1; switchSkQ(p1, nP1);
        T2 nP2; switchSkQ(p2, nP2);
        return { nP0, nP1, nP2 };
    }

    CubicSegment<T> toCubic() const {
        return {p0, p0 + 2.*(p1 - p0)/3., p2 + 2.*(p1 - p2)/3., p2};
    }
};

typedef QuadSegment<qreal> qQuadSegment1D;
typedef QuadSegment<float> SkQuadSegment1D;
typedef QuadSegment<QPointF> qQuadSegment2D;
typedef QuadSegment<SkPoint> SkQuadSegment2D;

#endif // QUADSEGMENT_H
