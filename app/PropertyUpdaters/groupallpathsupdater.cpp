#include "groupallpathsupdater.h"
#include "Boxes/groupbox.h"

GroupAllPathsUpdater::GroupAllPathsUpdater(
        GroupBox * const boxesGroup) : mTarget(boxesGroup) {}

void GroupAllPathsUpdater::update() {
    mTarget->updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void GroupAllPathsUpdater::frameChangeUpdate() {
    mTarget->updateAllChildPathBoxes(Animator::FRAME_CHANGE);
}
