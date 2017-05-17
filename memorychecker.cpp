#include "memorychecker.h"
#include <sys/sysinfo.h>
#define mb500 500000000ULL

MemoryChecker::MemoryChecker(QObject *parent) : QObject(parent) {
    struct sysinfo info;
    if(sysinfo(&info) == 0) {
        mMemUnit = info.mem_unit;
        mTotalRam = info.totalram*mMemUnit;
        mFreeRam = info.freeram*info.mem_unit +
                   //info.freeswap*info.mem_unit +
                   info.bufferram*info.mem_unit;
    } else { // ??
    }
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()),
            this, SLOT(checkMemory()) );
    mTimer->start(500);
}

void MemoryChecker::checkMemory() {
    struct sysinfo info;
    if(sysinfo(&info) == 0) {
        mFreeRam = info.freeram*info.mem_unit +
                   //info.freeswap*info.mem_unit +
                   info.bufferram*info.mem_unit;
        if(mFreeRam < mb500) {
            emit freeMemory(mb500 - mFreeRam);
        }
    }
}
