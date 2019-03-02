#include "groupallpathsupdater.h"
#include "Boxes/boxesgroup.h"

GroupAllPathsUpdater::GroupAllPathsUpdater(BoxesGroup *boxesGroup) {
    mTarget = boxesGroup;
}

void GroupAllPathsUpdater::update() {
    mTarget->updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void GroupAllPathsUpdater::frameChangeUpdate() {
    mTarget->updateAllChildPathBoxes(Animator::FRAME_CHANGE);
}
