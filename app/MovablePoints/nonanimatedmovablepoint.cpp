#include "nonanimatedmovablepoint.h"

NonAnimatedMovablePoint::NonAnimatedMovablePoint(
        BasicTransformAnimator * const parentTransform,
        const MovablePointType &type, const qreal &radius) :
    MovablePoint(parentTransform, type, radius) {}

void NonAnimatedMovablePoint::applyTransform(const QMatrix &transform) {
    mCurrentPos = transform.map(mCurrentPos);
}

void NonAnimatedMovablePoint::setRelativePos(const QPointF &relPos) {
    setRelativePosVal(relPos);
}

QPointF NonAnimatedMovablePoint::getRelativePos() const {
    return mCurrentPos;
}

void NonAnimatedMovablePoint::moveByRel(const QPointF &relTranslation) {
    setRelativePos(mSavedRelPos + relTranslation);
}

void NonAnimatedMovablePoint::cancelTransform() {
    setRelativePos(mSavedRelPos);
}
