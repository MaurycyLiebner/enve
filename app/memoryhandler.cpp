#include "memoryhandler.h"
#include "Boxes/boundingboxrendercontainer.h"
#include <gperftools/malloc_extension.h>
#include <malloc.h>
#include "GUI/mainwindow.h"
#include <QMetaType>
#include "GUI/usagewidget.h"

MemoryHandler *MemoryHandler::mInstance;
Q_DECLARE_METATYPE(MemoryState)

MemoryHandler::MemoryHandler(QObject *parent) : QObject(parent) {
    mInstance = this;

    mMemoryChekerThread = new QThread(this);
    mMemoryChecker = new MemoryChecker();
    mMemoryChecker->moveToThread(mMemoryChekerThread);
    qRegisterMetaType<MemoryState>();
    connect(mMemoryChecker,
            SIGNAL(handleMemoryState(const MemoryState &,
                                     const unsigned long long &)),
            this,
            SLOT(freeMemory(const MemoryState &,
                            const unsigned long long &)) );
    connect(mMemoryChecker,
            SIGNAL(memoryChecked(const int &, const int &)),
            this,
            SLOT(memoryChecked(const int &, const int &)) );

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()),
            mMemoryChecker, SLOT(checkMemory()) );
    mTimer->start(500);
    mMemoryChekerThread->start();
}

MemoryHandler::~MemoryHandler() {
    mMemoryChekerThread->quit();
    mMemoryChekerThread->wait();
    mInstance = nullptr;
    delete mMemoryChecker;
}

void MemoryHandler::addContainer(MinimalCacheContainer *cont) {
    if(cont->handledByMemoryHandler()) return;
    mContainers << cont;
    cont->setHandledByMemoryHanlder(true);
}

void MemoryHandler::removeContainer(MinimalCacheContainer *cont) {
    if(cont->handledByMemoryHandler()) {
        mContainers.removeOne(cont);
        cont->setHandledByMemoryHanlder(false);
    }
}

void MemoryHandler::containerUpdated(MinimalCacheContainer *cont) {
    removeContainer(cont);
    addContainer(cont);
}

void MemoryHandler::freeMemory(const MemoryState &state,
                               const unsigned long long &minFreeBytes) {
    if(state != mCurrentMemoryState) {
//        qDebug() << "set state: " << state;
        //bool worsend = state > mCurrentMemoryState;
        if(state == NORMAL_MEMORY_STATE) {
            disconnect(mTimer, nullptr, mMemoryChecker, nullptr);
            connect(mTimer, SIGNAL(timeout()),
                    mMemoryChecker, SLOT(checkMemory()) );
            mTimer->setInterval(1000);
        } else if(mCurrentMemoryState == NORMAL_MEMORY_STATE) {
            disconnect(mTimer, nullptr, mMemoryChecker, nullptr);
            connect(mTimer, SIGNAL(timeout()),
                    mMemoryChecker, SLOT(checkMajorMemoryPageFault()) );
            mTimer->setInterval(500);
        }
        mCurrentMemoryState = state;
    }

    long long memToFree = static_cast<long long>(minFreeBytes);
    if(state == LOW_MEMORY_STATE) {
        memToFree -= mMemoryScheduledToRemove;
    }
    if(memToFree <= 0) return;
    int unfreeable = 0;
    while(memToFree > 0 && mContainers.count() > unfreeable) {
        MinimalCacheContainer *cont = mContainers.takeFirst();
        int byteCount = cont->getByteCount();
        if(state == LOW_MEMORY_STATE) {
            if(cont->cacheAndFree()) {
                memToFree -= byteCount;
            } else {
                unfreeable++;
                mContainers << cont;
            }
        } else {
            if(cont->freeAndRemove()) {
                memToFree -= byteCount;
            } else {
                unfreeable++;
                mContainers << cont;
            }
        }
    }
    //emit allMemoryUsed();
    if(memToFree > 0 ||
        state == VERY_LOW_MEMORY_STATE ||
        state == CRITICAL_MEMORY_STATE) {
        emit allMemoryUsed();
    }
    emit memoryFreed();
    //MainWindow::getInstance()->callUpdateSchedulers();
    //MallocExtension::instance()->ReleaseToSystem(bytes - memToFree);
    //MallocExtension::instance()->ReleaseFreeMemory();
}

void MemoryHandler::memoryChecked(const int &memKb, const int& totMemKb) {
    UsageWidget* usageWidget = MainWindow::getInstance()->getUsageWidget();
    if(!usageWidget) return;
    MainWindow::getInstance()->getUsageWidget()->setTotalRam(totMemKb/1000000.);
    MainWindow::getInstance()->getUsageWidget()->setRamUsage(-memKb/1000000.);
}
