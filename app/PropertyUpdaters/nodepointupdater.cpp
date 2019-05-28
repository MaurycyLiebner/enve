#include "nodepointupdater.h"
#include "Boxes/pathbox.h"

NodePointUpdater::NodePointUpdater(PathBox * const pathBox) :
    mTarget(pathBox) {}

void NodePointUpdater::update() {
    if(!mTarget) return;
    mTarget->setPathsOutdated();
    mTarget->planScheduleUpdate(Animator::USER_CHANGE);
}

void NodePointUpdater::frameChangeUpdate() {
    if(!mTarget) return;
    mTarget->setPathsOutdated();
    mTarget->planScheduleUpdate(Animator::FRAME_CHANGE);
}
