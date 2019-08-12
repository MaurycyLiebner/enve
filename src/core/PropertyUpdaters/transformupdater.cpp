#include "transformupdater.h"
#include "Animators/transformanimator.h"

TransformUpdater::TransformUpdater(BasicTransformAnimator * const target) :
    mTarget(target) {}

void TransformUpdater::update() {
    mTarget->updateRelativeTransform(UpdateReason::userChange);
}

void TransformUpdater::finishedChange() {
    mTarget->updateRelativeTransform(UpdateReason::userChange);
}

void TransformUpdater::frameChangeUpdate() {
    mTarget->updateRelativeTransform(UpdateReason::frameChange);
}
