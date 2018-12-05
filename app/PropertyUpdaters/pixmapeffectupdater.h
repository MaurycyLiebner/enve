#ifndef PIXMAPEFFECTUPDATER_H
#define PIXMAPEFFECTUPDATER_H
#include "propertyupdater.h"
class BoundingBox;

class PixmapEffectUpdater : public PropertyUpdater {
public:
    PixmapEffectUpdater(BoundingBox *target);

    void update();
    void frameChangeUpdate();
    void updateFinal();
private:
    BoundingBox *mTarget;
};

#endif // PIXMAPEFFECTUPDATER_H
