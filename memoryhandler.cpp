#include "memoryhandler.h"
#include "Boxes/boundingboxrendercontainer.h"
#include <gperftools/malloc_extension.h>
#include <malloc.h>

MemoryHandler *MemoryHandler::mInstance;

MemoryHandler::MemoryHandler(QObject *parent) : QObject(parent) {
    mInstance = this;

    mMemoryChekerThread = new QThread(this);
    mMemoryChecker = new MemoryChecker();
    mMemoryChecker->moveToThread(mMemoryChekerThread);

    connect(mMemoryChecker, SIGNAL(outOfMemory(unsigned long long)),
            this, SLOT(freeMemory(unsigned long long)) );

    mMemoryChekerThread->start();
}

MemoryHandler::~MemoryHandler() {
    mMemoryChekerThread->quit();
    mMemoryChekerThread->wait();
    mInstance = NULL;
}

void MemoryHandler::addContainer(MinimalCacheContainer *cont) {
    mContainers << cont;
}

void MemoryHandler::removeContainer(MinimalCacheContainer *cont) {
    mContainers.removeOne(cont);
}

void MemoryHandler::containerUpdated(MinimalCacheContainer *cont) {
    removeContainer(cont);
    addContainer(cont);
}

void MemoryHandler::freeMemory(const unsigned long long &bytes) {
    long long memToFree = bytes;
    int unfreeable = 0;
    while(memToFree > 0 && mContainers.count() > unfreeable) {
        MinimalCacheContainer *cont = mContainers.takeFirst();
        int byteCount = cont->getByteCount();
        if(cont->freeThis()) {
            memToFree -= byteCount;
        } else {
            unfreeable++;
            mContainers << cont;
        }
    }
    if(memToFree > 0) {
        emit allMemoryUsed();
    }
    emit memoryFreed();

    //MallocExtension::instance()->ReleaseToSystem(bytes - memToFree);
//    MallocExtension::instance()->ReleaseFreeMemory();
}
