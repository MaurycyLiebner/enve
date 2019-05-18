#include "groupallpathsupdater.h"
#include "Boxes/layerbox.h"

GroupAllPathsUpdater::GroupAllPathsUpdater(LayerBox *boxesGroup) {
    mTarget = boxesGroup;
}

void GroupAllPathsUpdater::update() {
    mTarget->updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void GroupAllPathsUpdater::frameChangeUpdate() {
    mTarget->updateAllChildPathBoxes(Animator::FRAME_CHANGE);
}
