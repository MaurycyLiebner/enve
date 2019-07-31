#include "minimalcachecontainer.h"
#include "memorydatahandler.h"

CacheContainer::CacheContainer() {
    addToMemoryManagment();
}

CacheContainer::~CacheContainer() {
    if(!MemoryDataHandler::sInstance) return;
    removeFromMemoryManagment();
}

void CacheContainer::addToMemoryManagment() {
    if(mHandledByMemoryHandler || mBlocked) return;
    MemoryDataHandler::sInstance->addContainer(this);
    mHandledByMemoryHandler = true;
}

void CacheContainer::removeFromMemoryManagment() {
    if(!mHandledByMemoryHandler) return;
    MemoryDataHandler::sInstance->removeContainer(this);
    mHandledByMemoryHandler = false;
}

void CacheContainer::updateInMemoryManagment() {
    if(!mHandledByMemoryHandler) addToMemoryManagment();
    else MemoryDataHandler::sInstance->containerUpdated(this);
}
