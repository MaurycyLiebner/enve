#include "minimalcachecontainer.h"
#include "memoryhandler.h"

MinimalCacheContainer::MinimalCacheContainer() {
    MemoryHandler::getInstance()->addContainer(this);
}

MinimalCacheContainer::~MinimalCacheContainer() {
    if(!MemoryHandler::getInstance()) return;
    MemoryHandler::getInstance()->removeContainer(this);
}

bool MinimalCacheContainer::cacheFreeAndRemoveFromMemoryHandler() {
    if(cacheAndFree()) {
        MemoryHandler::getInstance()->removeContainer(this);
        return true;
    }
    return false;
}
