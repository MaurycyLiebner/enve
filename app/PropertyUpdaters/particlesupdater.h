#ifndef PARTICLESUPDATER_H
#define PARTICLESUPDATER_H
#include "propertyupdater.h"

class ParticleEmitter;
class ParticlesUpdater : public PropertyUpdater {
public:
    ParticlesUpdater(ParticleEmitter *target);

    void update();
    void frameChangeUpdate() {}
private:
    ParticleEmitter *mTarget;
};
#endif // PARTICLESUPDATER_H
