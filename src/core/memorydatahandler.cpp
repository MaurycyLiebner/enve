#include "memorydatahandler.h"

MemoryDataHandler *MemoryDataHandler::sInstance = nullptr;

MemoryDataHandler::MemoryDataHandler() {
    Q_ASSERT(!sInstance);
    sInstance = this;
}

void MemoryDataHandler::addContainer(CacheContainer * const cont) {
    mContainers << cont;
}

void MemoryDataHandler::removeContainer(CacheContainer * const cont) {
    mContainers.removeOne(cont);
}

void MemoryDataHandler::containerUpdated(CacheContainer * const cont) {
    removeContainer(cont);
    addContainer(cont);
}
