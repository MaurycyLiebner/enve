#include "pixmapeffectupdater.h"
#include "Boxes/boundingbox.h"

PixmapEffectUpdater::PixmapEffectUpdater(BoundingBox *target) {
    mTarget = target;
}

void PixmapEffectUpdater::update() {
    mTarget->planScheduleUpdate(UpdateReason::userChange);
}

void PixmapEffectUpdater::frameChangeUpdate() {
    mTarget->planScheduleUpdate(UpdateReason::frameChange);
}

void PixmapEffectUpdater::finishedChange() {
    mTarget->prp_afterWholeInfluenceRangeChanged();
}
