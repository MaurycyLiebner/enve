#ifndef NONANIMATEDMOVABLEPOINT_H
#define NONANIMATEDMOVABLEPOINT_H
#include "movablepoint.h"

class NonAnimatedMovablePoint : public MovablePoint {
    friend class StdSelfRef;
protected:
    NonAnimatedMovablePoint(BasicTransformAnimator * const parentTransform,
                            const MovablePointType &type,
                            const qreal &radius = 7.5);
public:
    QPointF getRelativePos() const final;
    void setRelativePos(const QPointF &relPos);
    void moveByRel(const QPointF &relTranslation) final;

    void cancelTransform();
    void applyTransform(const QMatrix &transform) final;
private:
    QPointF mCurrentPos;
};
#endif // NONANIMATEDMOVABLEPOINT_H
