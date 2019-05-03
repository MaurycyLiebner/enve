#include "displayedfillstrokesettingsupdater.h"
#include "Boxes/boundingbox.h"

DisplayedFillStrokeSettingsUpdater::
DisplayedFillStrokeSettingsUpdater(BoundingBox *path) {
    mTarget = path;
}

void DisplayedFillStrokeSettingsUpdater::update() {
    mTarget->planScheduleUpdate(Animator::USER_CHANGE);
}

void DisplayedFillStrokeSettingsUpdater::frameChangeUpdate() {
    mTarget->planScheduleUpdate(Animator::FRAME_CHANGE);
}

void DisplayedFillStrokeSettingsUpdater::finishedChange() {
    mTarget->prp_afterWholeInfluenceRangeChanged();
}
