#include "nonanimatedmovablepoint.h"

NonAnimatedMovablePoint::NonAnimatedMovablePoint(const MovablePointType &type) :
    MovablePoint(type) {}

NonAnimatedMovablePoint::NonAnimatedMovablePoint(
        BasicTransformAnimator * const trans,
        const MovablePointType &type) : NonAnimatedMovablePoint(type) {
    setTransform(trans);
}

void NonAnimatedMovablePoint::setRelativePos(const QPointF &relPos) {
    setValue(relPos);
}

QPointF NonAnimatedMovablePoint::getRelativePos() const {
    return getValue();
}

void NonAnimatedMovablePoint::cancelTransform() {
    setRelativePos(getSavedRelPos());
}
