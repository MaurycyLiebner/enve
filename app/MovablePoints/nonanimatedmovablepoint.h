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
    void setRelativePos(const QPointF &relPos);
    QPointF getRelativePos() const;
    void moveByRel(const QPointF &relTranslation);

    void cancelTransform();

    void applyTransform(const QMatrix &transform);

    void setRelativePosVal(const QPointF &relPos) {
        mCurrentPos = relPos;
    }
private:
    QPointF mCurrentPos;
};
#endif // NONANIMATEDMOVABLEPOINT_H
