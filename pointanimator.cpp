#include "pointanimator.h"

#include "Boxes/vectorpath.h"
#include "global.h"
#include "undoredo.h"
#include "Boxes/boxesgroup.h"
#include "skqtconversions.h"
#include "pointhelpers.h"

PointAnimator::PointAnimator(BasicTransformAnimator *parent,
                           const MovablePointType &type,
                           const qreal &radius) :
    QPointFAnimator(),
    MovablePoint(parent, type, radius) {
    mType = type;
    mRadius = radius;
    mParent = parent;
}

void PointAnimator::startTransform() {
    prp_startTransform();
    mTransformStarted = true;
    mSavedRelPos = getRelativePos();
}

void PointAnimator::applyTransform(const QMatrix &transform){
    QPointF point = getCurrentPointValue();
    setCurrentPointValue(transform.map(point), true);
}

void PointAnimator::removeAnimations() {
    if(prp_isRecording()) {
        prp_setRecording(false);
    }
}

void PointAnimator::finishTransform() {
    if(mTransformStarted) {
        mTransformStarted = false;
        prp_finishTransform();
    }
}

void PointAnimator::setRelativePos(const QPointF &relPos) {
    setCurrentPointValue(relPos);
}

QPointF PointAnimator::getRelativePos() const {
    return getCurrentPointValue();
}

QPointF PointAnimator::getRelativePosAtRelFrame(const int &frame) const {
    return getCurrentPointValueAtRelFrame(frame);
}


void PointAnimator::moveByRel(const QPointF &relTranslation) {
    incSavedValueToCurrentValue(relTranslation.x(),
                                relTranslation.y());
}

void PointAnimator::cancelTransform() {
    prp_cancelTransform();
    //setRelativePos(mSavedRelPos, false);
}

//void PointAnimator::prp_setUpdater(AnimatorUpdater *updater) {
//    prp_setUpdater(updater);
//}
