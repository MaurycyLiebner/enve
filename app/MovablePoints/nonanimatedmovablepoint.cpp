#include "nonanimatedmovablepoint.h"

NonAnimatedMovablePoint::NonAnimatedMovablePoint(
        BasicTransformAnimator * const parentTransform,
        const MovablePointType &type, const qreal &radius) :
    MovablePoint(parentTransform, type, radius) {}

void NonAnimatedMovablePoint::setRelativePos(const QPointF &relPos) {
    setValue(relPos);
}

QPointF NonAnimatedMovablePoint::getRelativePos() const {
    return getValue();
}

void NonAnimatedMovablePoint::cancelTransform() {
    setRelativePos(mSavedRelPos);
}
