#include "animationboxframeupdater.h"
#include "Boxes/animationbox.h"

AnimationBoxFrameUpdater::AnimationBoxFrameUpdater(AnimationBox *target) {
    mTarget = target;
}

void AnimationBoxFrameUpdater::update() {
    mTarget->planScheduleUpdate(UpdateReason::userChange);
    mTarget->updateDurationRectangleAnimationRange();
}
