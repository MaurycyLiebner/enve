#include "strokewidthupdater.h"
#include "Boxes/pathbox.h"

StrokeWidthUpdater::StrokeWidthUpdater(PathBox *path) {
    mTarget = path;
}

void StrokeWidthUpdater::update() {
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

void StrokeWidthUpdater::frameChangeUpdate() {
    mTarget->scheduleUpdate(Animator::FRAME_CHANGE);
}
