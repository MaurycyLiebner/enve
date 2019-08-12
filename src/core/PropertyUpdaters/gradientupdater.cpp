#include "gradientupdater.h"
#include "Animators/gradient.h"

GradientUpdater::GradientUpdater(Gradient *gradient) {
    mTarget = gradient;
}

void GradientUpdater::update() {
    mTarget->updateQGradientStops(UpdateReason::userChange);
}

void GradientUpdater::finishedChange() {
    mTarget->updateQGradientStops(UpdateReason::userChange);
}

void GradientUpdater::frameChangeUpdate() {
    mTarget->updateQGradientStops(UpdateReason::frameChange);
}
