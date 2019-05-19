#ifndef GROUPALLPATHSUPDATER_H
#define GROUPALLPATHSUPDATER_H
#include "PropertyUpdaters/propertyupdater.h"
class ContainerBox;

class GroupAllPathsUpdater : public PropertyUpdater {
public:
    GroupAllPathsUpdater(ContainerBox * const boxesGroup);
    void update();
    void frameChangeUpdate();
private:
    ContainerBox * const mTarget;
};

#endif // GROUPALLPATHSUPDATER_H
