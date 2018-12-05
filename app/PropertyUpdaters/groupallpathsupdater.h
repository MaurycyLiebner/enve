#ifndef GROUPALLPATHSUPDATER_H
#define GROUPALLPATHSUPDATER_H
#include "propertyupdater.h"
class BoxesGroup;

class GroupAllPathsUpdater : public PropertyUpdater {
public:
    GroupAllPathsUpdater(BoxesGroup *boxesGroup);
    void update();
    void frameChangeUpdate();
private:
    BoxesGroup *mTarget;
};

#endif // GROUPALLPATHSUPDATER_H
