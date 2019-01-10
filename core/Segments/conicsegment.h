#ifndef CONICSEGMENT_H
#define CONICSEGMENT_H
#include <skia/skiaincludes.h>
#include <QPointF>
template <typename T>
struct CubicSegment;

template <typename T>
struct QuadSegment;

template <typename T, typename Tw>
struct ConicSegment {
    T fP0;
    T fP1;
    T fP2;
    Tw fW;

    template <typename T2, typename T2w>
    explicit operator ConicSegment<T2, T2w> () const {
        T2 nP0; switchSkQ(fP0, nP0);
        T2 nP1; switchSkQ(fP1, nP1);
        T2 nP2; switchSkQ(fP2, nP2);
        T2w nW; switchSkQ(fW, nW);
        return { nP0, nP1, nP2, nW };
    }

//    QList<QuadSegment<T>> toQuads() const;

    CubicSegment<T> toCubic() const {
        SkScalar u = 4.f*fW/(3.f*(1.f + fW));
        return {fP0, fP0*(1.f - u) + fP1*u, fP2*(1.f - u) + fP1*u, fP2};
    }

    QuadSegment<T> toQuad() const {
        SkScalar u = 2.f*fW/(1.f + fW);
        return {fP0, (fP0 + fP2)*((1.f - u)/2.f) + fP1*u, fP2};
    }
};

typedef ConicSegment<qreal, qreal> qConicSegment1D;
typedef ConicSegment<SkScalar, SkScalar> SkConicSegment1D;
typedef ConicSegment<QPointF, qreal> qConicSegment2D;
typedef ConicSegment<SkPoint, SkScalar> SkConicSegment2D;

#endif // CONICSEGMENT_H
