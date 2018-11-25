#include "pointanimator.h"

#include "Boxes/vectorpath.h"
#include "global.h"
#include "undoredo.h"
#include "Boxes/boxesgroup.h"
#include "skqtconversions.h"
#include "pointhelpers.h"

PointAnimatorMovablePoint::PointAnimatorMovablePoint(
        QPointFAnimator *associatedAnimator,
        BasicTransformAnimator* parentTransform,
        const MovablePointType &type,
        const qreal &radius) :
    MovablePoint(parentTransform, type, radius),
    mAssociatedAnimator_k(associatedAnimator) {}

void PointAnimatorMovablePoint::startTransform() {
    mAssociatedAnimator_k->prp_startTransform();
    mTransformStarted = true;
    mSavedRelPos = getRelativePos();
}

void PointAnimatorMovablePoint::applyTransform(const QMatrix &transform){
    QPointF point = mAssociatedAnimator_k->getCurrentPointValue();
    mAssociatedAnimator_k->setCurrentPointValue(transform.map(point), true);
}

void PointAnimatorMovablePoint::finishTransform() {
    if(mTransformStarted) {
        mTransformStarted = false;
        mAssociatedAnimator_k->prp_finishTransform();
    }
}

void PointAnimatorMovablePoint::setRelativePos(const QPointF &relPos) {
    mAssociatedAnimator_k->setCurrentPointValue(relPos);
}

QPointF PointAnimatorMovablePoint::getRelativePos() const {
    return mAssociatedAnimator_k->getCurrentPointValue();
}

QPointF PointAnimatorMovablePoint::getRelativePosAtRelFrame(const int &frame) const {
    return mAssociatedAnimator_k->getCurrentPointValueAtRelFrame(frame);
}


void PointAnimatorMovablePoint::moveByRel(const QPointF &relTranslation) {
    mAssociatedAnimator_k->incSavedValueToCurrentValue(relTranslation.x(),
                                                 relTranslation.y());
}

void PointAnimatorMovablePoint::cancelTransform() {
    mAssociatedAnimator_k->prp_cancelTransform();
    //setRelativePos(mSavedRelPos, false);
}

//void PointAnimatorMovablePoint::prp_setUpdater(AnimatorUpdater *updater) {
//    prp_setUpdater(updater);
//}
