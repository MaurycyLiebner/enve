#include "animationboxframeupdater.h"
#include "Boxes/animationbox.h"

AnimationBoxFrameUpdater::AnimationBoxFrameUpdater(AnimationBox *target) {
    mTarget = target;
}

void AnimationBoxFrameUpdater::update() {
    mTarget->planScheduleUpdate(Animator::USER_CHANGE);
    mTarget->updateDurationRectangleAnimationRange();
}
