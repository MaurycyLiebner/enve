#ifndef PROPERTYUPDATER_H
#define PROPERTYUPDATER_H
#include "smartPointers/ememory.h"

class PropertyUpdater : public StdSelfRef {
public:
    PropertyUpdater();

    virtual void update();
    virtual void frameChangeUpdate();
    virtual void finishedChange();
};

#endif // PROPERTYUPDATER_H
