#include "strokewidthupdater.h"
#include "Boxes/pathbox.h"

StrokeWidthUpdater::StrokeWidthUpdater(PathBox *path) {
    mTarget = path;
}

void StrokeWidthUpdater::update() {
    mTarget->setOutlinePathOutdated();
    mTarget->planScheduleUpdate(UpdateReason::userChange);
}

void StrokeWidthUpdater::frameChangeUpdate() {
    mTarget->setOutlinePathOutdated();
    mTarget->planScheduleUpdate(UpdateReason::frameChange);
}
