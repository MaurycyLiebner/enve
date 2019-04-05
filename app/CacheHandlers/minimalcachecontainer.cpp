#include "minimalcachecontainer.h"
#include "memoryhandler.h"

MinimalCacheContainer::MinimalCacheContainer() {
    addToMemoryManagment();
}

MinimalCacheContainer::~MinimalCacheContainer() {
    if(!MemoryHandler::sGetInstance()) return;
    removeFromMemoryManagment();
}

void MinimalCacheContainer::addToMemoryManagment() {
    if(mHandledByMemoryHandler || mBlocked) return;
    MemoryHandler::sGetInstance()->addContainer(this);
    mHandledByMemoryHandler = true;
}

void MinimalCacheContainer::removeFromMemoryManagment() {
    if(!mHandledByMemoryHandler) return;
    MemoryHandler::sGetInstance()->removeContainer(this);
    mHandledByMemoryHandler = false;
}

void MinimalCacheContainer::updateInMemoryManagment() {
    if(!mHandledByMemoryHandler) addToMemoryManagment();
    else MemoryHandler::sGetInstance()->containerUpdated(this);
}
