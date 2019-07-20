#include "smartnodepointupdater.h"
#include "MovablePoints/pathpointshandler.h"

SmartNodePointUpdater::SmartNodePointUpdater(
        PathPointsHandler * const handler) : mHandler(handler) {}

void SmartNodePointUpdater::update() {
    mHandler->updateAllPoints();
}

void SmartNodePointUpdater::frameChangeUpdate() {
    mHandler->updateAllPoints();
}
