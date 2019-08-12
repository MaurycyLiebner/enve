#include "groupallpathsupdater.h"
#include "Boxes/containerbox.h"

GroupAllPathsUpdater::GroupAllPathsUpdater(
        ContainerBox * const boxesGroup) : mTarget(boxesGroup) {}

void GroupAllPathsUpdater::update() {
    mTarget->updateAllChildPathBoxes(UpdateReason::userChange);
}

void GroupAllPathsUpdater::frameChangeUpdate() {
    mTarget->updateAllChildPathBoxes(UpdateReason::frameChange);
}
