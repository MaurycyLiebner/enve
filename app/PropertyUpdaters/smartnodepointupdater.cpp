#include "smartnodepointupdater.h"
#include "Boxes/smartvectorpath.h"
#include "MovablePoints/pathpointshandler.h"

SmartNodePointUpdater::SmartNodePointUpdater(
        SmartVectorPath * const pathBox,
        PathPointsHandler * const handler) :
    NodePointUpdater(pathBox), mHandler(handler) {}

void SmartNodePointUpdater::update() {
    NodePointUpdater::update();
    mHandler->updateAllPoints();
}

void SmartNodePointUpdater::frameChangeUpdate() {
    NodePointUpdater::frameChangeUpdate();
    mHandler->updateAllPoints();
}
