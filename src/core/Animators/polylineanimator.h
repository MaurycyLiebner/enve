#ifndef POLYLINEANIMATOR_H
#define POLYLINEANIMATOR_H
#include "graphanimatort.h"
#include "polyline.h"

template <uchar PROPS>
class PolylineAnimator : public GraphAnimatorT<Polyline<PROPS>> {
    friend class SelfRef;
protected:
    PolylineAnimator() : GraphAnimatorT<Polyline<PROPS>>("polyline") {}
public:
};

#endif // POLYLINEANIMATOR_H
