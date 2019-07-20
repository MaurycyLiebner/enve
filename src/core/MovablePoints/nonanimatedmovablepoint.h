#ifndef NONANIMATEDMOVABLEPOINT_H
#define NONANIMATEDMOVABLEPOINT_H
#include "movablepoint.h"

class NonAnimatedMovablePoint : public MovablePoint {
    friend class StdSelfRef;
protected:
    NonAnimatedMovablePoint(const MovablePointType &type);
    NonAnimatedMovablePoint(BasicTransformAnimator * const trans,
                            const MovablePointType &type);
public:
    QPointF getRelativePos() const;
    void setRelativePos(const QPointF &relPos);

    void cancelTransform();

    QPointF getValue() const { return mValue; }
    void setValue(const QPointF& value) { mValue = value; }
private:
    QPointF mValue;
};
#endif // NONANIMATEDMOVABLEPOINT_H
