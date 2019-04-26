#include "pixmapeffectupdater.h"
#include "Boxes/boundingbox.h"

PixmapEffectUpdater::PixmapEffectUpdater(BoundingBox *target) {
    mTarget = target;
}

void PixmapEffectUpdater::update() {
    mTarget->planScheduleUpdate(Animator::USER_CHANGE);
}

void PixmapEffectUpdater::frameChangeUpdate() {
    mTarget->planScheduleUpdate(Animator::FRAME_CHANGE);
}

void PixmapEffectUpdater::finishedChange() {
    mTarget->prp_updateInfluenceRangeAfterChanged();
}
