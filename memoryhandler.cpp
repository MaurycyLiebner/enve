#include "memoryhandler.h"
#include "Boxes/boundingboxrendercontainer.h"
MemoryHandler *MemoryHandler::mInstance;

MemoryHandler::MemoryHandler(QObject *parent) : QObject(parent) {
    mInstance = this;
    mMemoryChekerThread = new QThread(this);
    mMemoryChecker = new MemoryChecker();
    mMemoryChecker->moveToThread(mMemoryChekerThread);

    connect(mMemoryChecker, SIGNAL(freeMemory(ulong)),
            this, SLOT(freeMemory(ulong)) );

    mMemoryChekerThread->start();
}

MemoryHandler::~MemoryHandler() {
    mMemoryChekerThread->quit();
    mMemoryChekerThread->wait();
}

void MemoryHandler::addContainer(CacheBoundingBoxRenderContainer *cont) {
    mContainers << cont;
}

void MemoryHandler::removeContainer(CacheBoundingBoxRenderContainer *cont) {
    mContainers.removeOne(cont);
}

void MemoryHandler::containerUpdated(CacheBoundingBoxRenderContainer *cont) {
    removeContainer(cont);
    addContainer(cont);
}

void MemoryHandler::freeMemory(const ulong &bytes) {
    long memToFree = bytes;
    while(memToFree > 0 && !mContainers.isEmpty()) {
        CacheBoundingBoxRenderContainer *cont = mContainers.takeFirst();
        memToFree -= cont->getByteCount();
        cont->freeThis();
    }
}
