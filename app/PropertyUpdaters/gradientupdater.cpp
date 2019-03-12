#include "gradientupdater.h"
#include "Animators/gradient.h"

GradientUpdater::GradientUpdater(Gradient *gradient) {
    mTarget = gradient;
}

void GradientUpdater::update() {
    mTarget->updateQGradientStops(Animator::USER_CHANGE);
}

void GradientUpdater::finishedChange() {
    mTarget->updateQGradientStops(Animator::USER_CHANGE);
}

void GradientUpdater::frameChangeUpdate() {
    mTarget->updateQGradientStops(Animator::FRAME_CHANGE);
}
