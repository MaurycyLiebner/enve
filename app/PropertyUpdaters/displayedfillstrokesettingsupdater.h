#ifndef DISPLAYEDFILLSTROKESETTINGSUPDATER_H
#define DISPLAYEDFILLSTROKESETTINGSUPDATER_H
#include "propertyupdater.h"
class BoundingBox;

class DisplayedFillStrokeSettingsUpdater : public PropertyUpdater {
public:
    DisplayedFillStrokeSettingsUpdater(BoundingBox *path);

    void update();
    void frameChangeUpdate();
    void updateFinal();
private:
    BoundingBox *mTarget;
};

#endif // DISPLAYEDFILLSTROKESETTINGSUPDATER_H
