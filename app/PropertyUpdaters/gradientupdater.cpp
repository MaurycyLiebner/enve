#include "gradientupdater.h"
#include "Animators/paintsettings.h"

GradientUpdater::GradientUpdater(Gradient *gradient) {
    mTarget = gradient;
}

void GradientUpdater::update() {
    mTarget->updateQGradientStopsFinal(Animator::USER_CHANGE);
}

void GradientUpdater::updateFinal() {
    mTarget->updateQGradientStopsFinal(Animator::USER_CHANGE);
}

void GradientUpdater::frameChangeUpdate() {
    mTarget->updateQGradientStops(Animator::FRAME_CHANGE);
}
