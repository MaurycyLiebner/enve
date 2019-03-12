#include "animatedpoint.h"

#include "Boxes/vectorpath.h"
#include "global.h"
#include "undoredo.h"
#include "Boxes/boxesgroup.h"
#include "skia/skqtconversions.h"
#include "pointhelpers.h"

AnimatedPoint::AnimatedPoint(
        QPointFAnimator *associatedAnimator,
        BasicTransformAnimator* parentTransform,
        const MovablePointType &type,
        const qreal &radius) :
    MovablePoint(parentTransform, type, radius),
    mAssociatedAnimator_k(associatedAnimator) {}

void AnimatedPoint::startTransform() {
    mAssociatedAnimator_k->prp_startTransform();
    mTransformStarted = true;
    mSavedRelPos = getRelativePos();
}

void AnimatedPoint::applyTransform(const QMatrix &transform) {
    QPointF point = mAssociatedAnimator_k->getCurrentPointValue();
    mAssociatedAnimator_k->setCurrentPointValue(transform.map(point));
}

void AnimatedPoint::finishTransform() {
    if(mTransformStarted) {
        mTransformStarted = false;
        mAssociatedAnimator_k->prp_finishTransform();
    }
}

void AnimatedPoint::setRelativePos(const QPointF &relPos) {
    mAssociatedAnimator_k->setCurrentPointValue(relPos);
}

QPointF AnimatedPoint::getRelativePos() const {
    return mAssociatedAnimator_k->getCurrentPointValue();
}

QPointF AnimatedPoint::getRelativePosAtRelFrame(const qreal &frame) const {
    return mAssociatedAnimator_k->getCurrentPointValueAtRelFrame(frame);
}


void AnimatedPoint::moveByRel(const QPointF &relTranslation) {
    mAssociatedAnimator_k->incSavedValueToCurrentValue(relTranslation.x(),
                                                 relTranslation.y());
}

void AnimatedPoint::cancelTransform() {
    mAssociatedAnimator_k->prp_cancelTransform();
    //setRelativePos(mSavedRelPos, false);
}

//void AnimatedPoint::prp_setInheritedUpdater(AnimatorUpdater *updater) {
//    prp_setInheritedUpdater(updater);
//}
