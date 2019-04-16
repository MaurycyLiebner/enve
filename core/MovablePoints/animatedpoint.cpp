#include "animatedpoint.h"
#include "skia/skqtconversions.h"
#include "pointhelpers.h"

AnimatedPoint::AnimatedPoint(
        QPointFAnimator * const associatedAnimator,
        BasicTransformAnimator* const parentTransform,
        const MovablePointType &type,
        const qreal &radius) :
    MovablePoint(parentTransform, type, radius),
    mAssociatedAnimator_k(associatedAnimator) {}

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
