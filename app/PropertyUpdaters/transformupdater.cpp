#include "transformupdater.h"
#include "Animators/transformanimator.h"

TransformUpdater::TransformUpdater(BasicTransformAnimator * const target) :
    mTarget(target) {}

void TransformUpdater::update() {
    mTarget->updateRelativeTransform(Animator::USER_CHANGE);
}

void TransformUpdater::finishedChange() {
    mTarget->updateRelativeTransform(Animator::USER_CHANGE);
}

void TransformUpdater::frameChangeUpdate() {
    mTarget->updateRelativeTransform(Animator::FRAME_CHANGE);
}
