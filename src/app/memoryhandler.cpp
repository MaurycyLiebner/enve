#include "memoryhandler.h"
#include "Boxes/boxrendercontainer.h"
#include <gperftools/malloc_extension.h>
#include <malloc.h>
#include "GUI/mainwindow.h"
#include <QMetaType>
#include "GUI/usagewidget.h"

MemoryHandler *MemoryHandler::sInstance = nullptr;
Q_DECLARE_METATYPE(MemoryState)

MemoryHandler::MemoryHandler(QObject * const parent) : QObject(parent) {
    Q_ASSERT(!sInstance);
    sInstance = this;

    mMemoryChekerThread = new QThread(this);
    mMemoryChecker = new MemoryChecker();
    mMemoryChecker->moveToThread(mMemoryChekerThread);
    qRegisterMetaType<MemoryState>();
    connect(mMemoryChecker, &MemoryChecker::handleMemoryState,
            this, &MemoryHandler::freeMemory);
    connect(mMemoryChecker, &MemoryChecker::memoryCheckedKB,
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

void MemoryHandler::freeMemory(const MemoryState &state,
                               const long &minFreeBytes) {
    if(state != mCurrentMemoryState) {
        if(state == NORMAL_MEMORY_STATE) {
            mTimer->setInterval(1000);
        } else if(mCurrentMemoryState == NORMAL_MEMORY_STATE) {
            mTimer->setInterval(500);
        }
        mCurrentMemoryState = state;
    }

    if(minFreeBytes <= 0) return;
    long memToFree = minFreeBytes;
    while(memToFree > 0 && !mDataHandler.isEmpty()) {
        const auto cont = mDataHandler.takeFirst();
        memToFree -= cont->free_RAM_k();
    }
    if(memToFree > 0 || state > LOW_MEMORY_STATE) emit allMemoryUsed();
    emit memoryFreed();
}

void MemoryHandler::memoryChecked(const int memKb, const int totMemKb) {
    const auto usageWidget = MainWindow::sGetInstance()->getUsageWidget();
    if(!usageWidget) return;
    usageWidget->setTotalRam(totMemKb/1000000.);
    usageWidget->setRamUsage((totMemKb - memKb)/1000000.);
}
