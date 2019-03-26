#ifndef SMARTNODEPOINTUPDATER_H
#define SMARTNODEPOINTUPDATER_H
#include "PropertyUpdaters/propertyupdater.h"
class PathPointsHandler;
class SmartVectorPath;

class SmartNodePointUpdater : public PropertyUpdater {
public:
    SmartNodePointUpdater(PathPointsHandler * const handler);

    void update();
    void frameChangeUpdate();
private:
    PathPointsHandler * const mHandler;
};

#endif // SMARTNODEPOINTUPDATER_H
