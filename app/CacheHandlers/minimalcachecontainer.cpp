#include "minimalcachecontainer.h"
#include "memoryhandler.h"

MinimalCacheContainer::MinimalCacheContainer() {
    MemoryHandler::sGetInstance()->addContainer(this);
}

MinimalCacheContainer::~MinimalCacheContainer() {
    if(!MemoryHandler::sGetInstance()) return;
    MemoryHandler::sGetInstance()->removeContainer(this);
}

bool MinimalCacheContainer::cacheFreeAndRemoveFromMemoryHandler() {
    if(cacheAndFree()) {
        MemoryHandler::sGetInstance()->removeContainer(this);
        return true;
    }
    return false;
}
