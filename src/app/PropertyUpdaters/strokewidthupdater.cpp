#include "strokewidthupdater.h"
#include "Boxes/pathbox.h"

StrokeWidthUpdater::StrokeWidthUpdater(PathBox *path) {
    mTarget = path;
}

void StrokeWidthUpdater::update() {
    mTarget->planScheduleUpdate(Animator::USER_CHANGE);
    mTarget->setOutlinePathOutdated();
}

void StrokeWidthUpdater::frameChangeUpdate() {
    mTarget->planScheduleUpdate(Animator::FRAME_CHANGE);
}
