#include "animatedpoint.h"

#include "global.h"
#include "undoredo.h"
#include "Boxes/boxesgroup.h"
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
    MovablePoint::startTransform();
    mAssociatedAnimator_k->prp_startTransform();
}

void AnimatedPoint::finishTransform() {
    if(mTransformStarted) mAssociatedAnimator_k->prp_finishTransform();
    MovablePoint::finishTransform();
}

void AnimatedPoint::cancelTransform() {
    mAssociatedAnimator_k->prp_cancelTransform();
}
