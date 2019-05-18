#ifndef GROUPALLPATHSUPDATER_H
#define GROUPALLPATHSUPDATER_H
#include "PropertyUpdaters/propertyupdater.h"
class LayerBox;

class GroupAllPathsUpdater : public PropertyUpdater {
public:
    GroupAllPathsUpdater(LayerBox *boxesGroup);
    void update();
    void frameChangeUpdate();
private:
    LayerBox *mTarget;
};

#endif // GROUPALLPATHSUPDATER_H
