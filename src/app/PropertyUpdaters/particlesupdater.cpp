#include "particlesupdater.h"
#include "Boxes/particlebox.h"

ParticlesUpdater::ParticlesUpdater(ParticleEmitter *target) {
    mTarget = target;
}

void ParticlesUpdater::update() {
    mTarget->scheduleGenerateParticles();
}
