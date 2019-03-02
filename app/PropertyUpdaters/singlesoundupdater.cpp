#include "singlesoundupdater.h"
#include "Sound/singlesound.h"

SingleSoundUpdater::SingleSoundUpdater(SingleSound *sound) :
    PropertyUpdater() {
    mTarget = sound;
}

SingleSoundUpdater::~SingleSoundUpdater() {
}

void SingleSoundUpdater::update() {
    mTarget->scheduleFinalDataUpdate();
}
