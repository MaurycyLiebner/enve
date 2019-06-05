#ifndef POLYLINEPOINT_H
#define POLYLINEPOINT_H
#include "MovablePoints/movablepoint.h"
#include "polyline.h"

template <uchar PROPS>
class PolylinePointBase : public MovablePoint {
    friend class StdSelfRef;
protected:
    PolylinePointBase(const int id, Polyline<PROPS> * poly) :
        MovablePoint(TYPE_PATH_POINT), mId(id), mPoly(poly) {}
public:
    QPointF getRelativePos() const {
        return getPosRef();
    }

    void set(const int id, Polyline<PROPS> * poly) {
        mId = id;
        mPoly = poly;
    }
protected:
    QPointF& getPosRef() const {
        return reinterpret_cast<QPointF&>(*(mPoly->begin() + mId));
    }
private:
    int mId;
    Polyline<PROPS> * mPoly;
};

#endif // POLYLINEPOINT_H
