#ifndef BRUSHPOLYPOINT_H
#define BRUSHPOLYPOINT_H
#include "polylinepoint.h"
class BrushPolylineAnimator;

class BrushPolyPoint : public PolylinePointBase<3> {
    e_OBJECT
protected:
    BrushPolyPoint(const int id, Polyline<3> * poly,
                   BrushPolylineAnimator * const polyAnim) :
        PolylinePointBase(id, poly), mPolyAnim(polyAnim) {}
public:
    void setRelativePos(const QPointF& pos);

    void startTransform();
    void cancelTransform();
    void finishTransform();
private:
    BrushPolylineAnimator * const mPolyAnim;
};

#endif // BRUSHPOLYPOINT_H
