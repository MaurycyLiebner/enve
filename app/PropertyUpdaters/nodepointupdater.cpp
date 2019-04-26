#include "nodepointupdater.h"
#include "Boxes/pathbox.h"

NodePointUpdater::NodePointUpdater(PathBox * const pathBox) :
    mTarget(pathBox) {}

void NodePointUpdater::update() {
    if(!mTarget) return;
    mTarget->planScheduleUpdate(Animator::USER_CHANGE);
    mTarget->setPathsOutdated();
}

void NodePointUpdater::frameChangeUpdate() {
    if(!mTarget) return;
    mTarget->planScheduleUpdate(Animator::FRAME_CHANGE);
    mTarget->setPathsOutdated();
}
