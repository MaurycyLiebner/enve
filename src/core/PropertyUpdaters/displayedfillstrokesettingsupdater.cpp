#include "displayedfillstrokesettingsupdater.h"
#include "Boxes/boundingbox.h"

DisplayedFillStrokeSettingsUpdater::
DisplayedFillStrokeSettingsUpdater(BoundingBox *path) {
    mTarget = path;
}

void DisplayedFillStrokeSettingsUpdater::update() {
    mTarget->planScheduleUpdate(UpdateReason::userChange);
}

void DisplayedFillStrokeSettingsUpdater::frameChangeUpdate() {
    mTarget->planScheduleUpdate(UpdateReason::frameChange);
}

void DisplayedFillStrokeSettingsUpdater::finishedChange() {
    mTarget->prp_afterWholeInfluenceRangeChanged();
}
