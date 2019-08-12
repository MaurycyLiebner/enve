#include "nodepointupdater.h"
#include "Boxes/pathbox.h"

NodePointUpdater::NodePointUpdater(PathBox * const pathBox) :
    mTarget(pathBox) {}

void NodePointUpdater::update() {
    if(!mTarget) return;
    mTarget->setPathsOutdated();
    mTarget->planScheduleUpdate(UpdateReason::userChange);
}

void NodePointUpdater::frameChangeUpdate() {
    if(!mTarget) return;
    mTarget->setPathsOutdated();
    mTarget->planScheduleUpdate(UpdateReason::frameChange);
}
