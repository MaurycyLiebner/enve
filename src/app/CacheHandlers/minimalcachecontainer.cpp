#include "minimalcachecontainer.h"
#include "memoryhandler.h"

CacheContainer::CacheContainer() {
    addToMemoryManagment();
}

CacheContainer::~CacheContainer() {
    if(!MemoryHandler::sGetInstance()) return;
    removeFromMemoryManagment();
}

void CacheContainer::addToMemoryManagment() {
    if(mHandledByMemoryHandler || mBlocked) return;
    MemoryHandler::sGetInstance()->addContainer(this);
    mHandledByMemoryHandler = true;
}

void CacheContainer::removeFromMemoryManagment() {
    if(!mHandledByMemoryHandler) return;
    MemoryHandler::sGetInstance()->removeContainer(this);
    mHandledByMemoryHandler = false;
}

void CacheContainer::updateInMemoryManagment() {
    if(!mHandledByMemoryHandler) addToMemoryManagment();
    else MemoryHandler::sGetInstance()->containerUpdated(this);
}
