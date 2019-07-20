#include "animatedpoint.h"
#include "skia/skqtconversions.h"
#include "pointhelpers.h"

AnimatedPoint::AnimatedPoint(
        QPointFAnimator * const associatedAnimator,
        const MovablePointType &type) :
    MovablePoint(type),
    mAssociatedAnimator_k(associatedAnimator) {}

AnimatedPoint::AnimatedPoint(QPointFAnimator * const associatedAnimator,
                             BasicTransformAnimator * const trans,
                             const MovablePointType &type) :
    AnimatedPoint(associatedAnimator, type) {
    setTransform(trans);
}

void AnimatedPoint::setRelativePos(const QPointF &relPos) {
    setValue(relPos);
}

QPointF AnimatedPoint::getRelativePos() const {
    return getValue();
}

void AnimatedPoint::startTransform() {
    mAssociatedAnimator_k->prp_startTransform();
    MovablePoint::startTransform();
}

void AnimatedPoint::finishTransform() {
    mAssociatedAnimator_k->prp_finishTransform();
}

void AnimatedPoint::cancelTransform() {
    mAssociatedAnimator_k->prp_cancelTransform();
}
