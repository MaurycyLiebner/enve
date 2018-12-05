#include "pixmapeffectupdater.h"
#include "Boxes/boundingbox.h"

PixmapEffectUpdater::PixmapEffectUpdater(BoundingBox *target) {
    mTarget = target;
}

void PixmapEffectUpdater::update() {
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

void PixmapEffectUpdater::frameChangeUpdate() {
    mTarget->scheduleUpdate(Animator::FRAME_CHANGE);
}

void PixmapEffectUpdater::updateFinal() {
    mTarget->clearAllCache();
}
