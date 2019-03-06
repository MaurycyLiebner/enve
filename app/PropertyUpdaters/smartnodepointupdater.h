#ifndef SMARTNODEPOINTUPDATER_H
#define SMARTNODEPOINTUPDATER_H
#include "nodepointupdater.h"
class PathPointsHandler;
class SmartVectorPath;

class SmartNodePointUpdater : public NodePointUpdater {
public:
    SmartNodePointUpdater(SmartVectorPath * const pathBox,
                          PathPointsHandler * const handler);

    void update();
    void frameChangeUpdate();
private:
    PathPointsHandler * const mHandler;
};

#endif // SMARTNODEPOINTUPDATER_H
