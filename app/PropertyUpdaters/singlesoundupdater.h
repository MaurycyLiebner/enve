#ifndef SINGLESOUNDUPDATER_H
#define SINGLESOUNDUPDATER_H
#include "propertyupdater.h"
class SingleSound;

class SingleSoundUpdater : public PropertyUpdater {
public:
    SingleSoundUpdater(SingleSound *sound);
    ~SingleSoundUpdater();

    void update();

    void frameChangeUpdate() {}
private:
    SingleSound *mTarget;
};
#endif // SINGLESOUNDUPDATER_H
