#include "transformupdater.h"
#include "Animators/transformanimator.h"

TransformUpdater::TransformUpdater(BasicTransformAnimator *transformAnimator) {
    mTarget = transformAnimator;
}

void TransformUpdater::update() {
    mTarget->updateRelativeTransform(Animator::USER_CHANGE);
}

void TransformUpdater::updateFinal() {
    mTarget->updateRelativeTransform(Animator::USER_CHANGE);
}

void TransformUpdater::frameChangeUpdate() {
    mTarget->updateRelativeTransform(Animator::FRAME_CHANGE);
}
