#include "memoryhandler.h"
#include "Boxes/boundingboxrendercontainer.h"
#include <gperftools/malloc_extension.h>
#include <malloc.h>
#include "GUI/mainwindow.h"
#include <QMetaType>
#include "GUI/usagewidget.h"

MemoryHandler *MemoryHandler::sInstance;
Q_DECLARE_METATYPE(MemoryState)

MemoryHandler::MemoryHandler(QObject * const parent) : QObject(parent) {
    sInstance = this;

    mMemoryChekerThread = new QThread(this);
    mMemoryChecker = new MemoryChecker();
    mMemoryChecker->moveToThread(mMemoryChekerThread);
    qRegisterMetaType<MemoryState>();
    connect(mMemoryChecker, &MemoryChecker::handleMemoryState,
            this, &MemoryHandler::freeMemory);
    connect(mMemoryChecker, &MemoryChecker::memoryChecked,
            this, &MemoryHandler::memoryChecked);

    mTimer = new QTimer(this);
    connect(mTimer, &QTimer::timeout,
            mMemoryChecker, &MemoryChecker::checkMemory);
    mTimer->start(1000);
    mMemoryChekerThread->start();
}

MemoryHandler::~MemoryHandler() {
    mMemoryChekerThread->quit();
    mMemoryChekerThread->wait();
    sInstance = nullptr;
    delete mMemoryChecker;
}

void MemoryHandler::addContainer(CacheContainer * const cont) {
    mContainers << cont;
}

void MemoryHandler::removeContainer(CacheContainer * const cont) {
    mContainers.removeOne(cont);
}

void MemoryHandler::containerUpdated(CacheContainer * const cont) {
    removeContainer(cont);
    addContainer(cont);
}

void MemoryHandler::freeMemory(const MemoryState &state,
                               const unsigned long long &minFreeBytes) {
    if(state != mCurrentMemoryState) {
        if(state == NORMAL_MEMORY_STATE) {
            mTimer->setInterval(1000);
        } else if(mCurrentMemoryState == NORMAL_MEMORY_STATE) {
            mTimer->setInterval(500);
        }
        mCurrentMemoryState = state;
    }

    long long memToFree = static_cast<long long>(minFreeBytes);
    if(memToFree <= 0) return;
    while(memToFree > 0 && !mContainers.isEmpty()) {
        const auto cont = mContainers.takeFirst();
        memToFree -= cont->free_RAM_k();
    }
    if(memToFree > 0 || state > LOW_MEMORY_STATE) emit allMemoryUsed();
    emit memoryFreed();
}

void MemoryHandler::memoryChecked(const int &memKb, const int& totMemKb) {
    const auto usageWidget = MainWindow::getInstance()->getUsageWidget();
    if(!usageWidget) return;
    usageWidget->setTotalRam(totMemKb/1000000.);
    usageWidget->setRamUsage(-memKb/1000000.);
}
