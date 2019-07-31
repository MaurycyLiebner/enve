#include "strokewidthupdater.h"
#include "Boxes/pathbox.h"

StrokeWidthUpdater::StrokeWidthUpdater(PathBox *path) {
    mTarget = path;
}

void StrokeWidthUpdater::update() {
    mTarget->setOutlinePathOutdated();
    mTarget->planScheduleUpdate(Animator::USER_CHANGE);
}

void StrokeWidthUpdater::frameChangeUpdate() {
    mTarget->setOutlinePathOutdated();
    mTarget->planScheduleUpdate(Animator::FRAME_CHANGE);
}
