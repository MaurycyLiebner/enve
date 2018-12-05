#ifndef RANDOMQREALGENERATORUPDATER_H
#define RANDOMQREALGENERATORUPDATER_H
#include "propertyupdater.h"
class RandomQrealGenerator;

class RandomQrealGeneratorUpdater : public PropertyUpdater {
public:
    RandomQrealGeneratorUpdater(RandomQrealGenerator *target);
    void update();
    void frameChangeUpdate() {}
private:
    RandomQrealGenerator *mTarget;
};
#endif // RANDOMQREALGENERATORUPDATER_H
