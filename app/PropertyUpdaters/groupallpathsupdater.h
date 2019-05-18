#ifndef GROUPALLPATHSUPDATER_H
#define GROUPALLPATHSUPDATER_H
#include "PropertyUpdaters/propertyupdater.h"
class GroupBox;

class GroupAllPathsUpdater : public PropertyUpdater {
public:
    GroupAllPathsUpdater(GroupBox *boxesGroup);
    void update();
    void frameChangeUpdate();
private:
    GroupBox * const mTarget;
};

#endif // GROUPALLPATHSUPDATER_H
