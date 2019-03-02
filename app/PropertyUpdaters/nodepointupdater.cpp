#include "nodepointupdater.h"
#include "Boxes/pathbox.h"

NodePointUpdater::NodePointUpdater(PathBox *vectorPath) {
    mTarget = vectorPath;
}

void NodePointUpdater::update() {
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
    mTarget->setPathsOutdated();
}

void NodePointUpdater::frameChangeUpdate() {
    mTarget->scheduleUpdate(Animator::FRAME_CHANGE);
    mTarget->setPathsOutdated();
}
