#include "gradientpointsupdater.h"
#include "Boxes/pathbox.h"

GradientPointsUpdater::GradientPointsUpdater(const bool &isFill,
                                             PathBox *target) :
    PropertyUpdater() {
    mTarget = target;
    mIsFill = isFill;
}

void GradientPointsUpdater::update() {
    mTarget->planScheduleUpdate(Animator::USER_CHANGE);
    if(mIsFill) mTarget->updateFillDrawGradient();
    else mTarget->updateStrokeDrawGradient();
    mTarget->planScheduleUpdate(Animator::USER_CHANGE);
}

void GradientPointsUpdater::frameChangeUpdate() {
    if(mIsFill) mTarget->updateFillDrawGradient();
    else mTarget->updateStrokeDrawGradient();
    mTarget->planScheduleUpdate(Animator::FRAME_CHANGE);
}
