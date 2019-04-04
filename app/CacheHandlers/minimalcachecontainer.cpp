#include "minimalcachecontainer.h"
#include "memoryhandler.h"

MinimalCacheContainer::MinimalCacheContainer() {
    MemoryHandler::sGetInstance()->addContainer(this);
}

MinimalCacheContainer::~MinimalCacheContainer() {
    if(!MemoryHandler::sGetInstance()) return;
    MemoryHandler::sGetInstance()->removeContainer(this);
}
