#ifndef PROPERTYUPDATER_H
#define PROPERTYUPDATER_H
#include "smartPointers/sharedpointerdefs.h"

class PropertyUpdater : public StdSelfRef {
public:
    PropertyUpdater();
    virtual ~PropertyUpdater();

    virtual void update();
    virtual void frameChangeUpdate();
    virtual void finishedChange();
};

#endif // PROPERTYUPDATER_H
