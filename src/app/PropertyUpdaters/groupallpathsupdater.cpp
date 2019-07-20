#include "groupallpathsupdater.h"
#include "Boxes/containerbox.h"

GroupAllPathsUpdater::GroupAllPathsUpdater(
        ContainerBox * const boxesGroup) : mTarget(boxesGroup) {}

void GroupAllPathsUpdater::update() {
    mTarget->updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void GroupAllPathsUpdater::frameChangeUpdate() {
    mTarget->updateAllChildPathBoxes(Animator::FRAME_CHANGE);
}
