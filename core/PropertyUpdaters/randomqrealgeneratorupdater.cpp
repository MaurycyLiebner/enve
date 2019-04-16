#include "randomqrealgeneratorupdater.h"
#include "Animators/randomqrealgenerator.h"

RandomQrealGeneratorUpdater::RandomQrealGeneratorUpdater(
        RandomQrealGenerator *target) {
    mTarget = target;
}

void RandomQrealGeneratorUpdater::update() {
    mTarget->generateData();
}
