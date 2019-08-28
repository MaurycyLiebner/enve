// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
    if(memToFree > 0/* || state == VERY_LOW_MEMORY_STATE*/) emit allMemoryUsed();
    emit memoryFreed();
}

void MemoryHandler::memoryChecked(const int memKb, const int totMemKb) {
    if(!MainWindow::sGetInstance()) return;
    const auto usageWidget = MainWindow::sGetInstance()->getUsageWidget();
    if(!usageWidget) return;
    usageWidget->setTotalRam(totMemKb/1000000.);
    usageWidget->setRamUsage((totMemKb - memKb)/1000000.);
}
