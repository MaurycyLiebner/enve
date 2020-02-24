// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#include "memorychecker.h"

#include <sys/sysinfo.h>
#include <gperftools/tcmalloc.h>
#include <gperftools/malloc_extension.h>
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <unistd.h>

#include "exceptions.h"
#include "hardwareinfo.h"
MemoryChecker *MemoryChecker::mInstance;

MemoryChecker::MemoryChecker(QObject * const parent) : QObject(parent) {
    mInstance = this;

    const intKB totRam = HardwareInfo::sRamKB();
    mLowFreeKB = totRam*20/100;
    mVeryLowFreeKB = totRam*15/100;
    mCriticalFreeKB = totRam*10/100;
}

char MemoryChecker::sLine[256];

void MemoryChecker::sGetFreeKB(intKB& procFreeKB, intKB& sysFreeKB) {
//    qDebug() << "";
    size_t virtual_memory_used;
    size_t physical_memory_used;
    size_t bytes_in_use_by_app;
    MallocExtension::instance()->eMemoryStats(&virtual_memory_used,
                                              &physical_memory_used,
                                              &bytes_in_use_by_app);

//    qDebug() << "virtual_memory_used" << intMB(longB(virtual_memory_used)).fValue;
//    qDebug() << "physical_memory_used" << intMB(longB(physical_memory_used)).fValue;
//    qDebug() << "bytes_in_use_by_app" << intMB(longB(bytes_in_use_by_app)).fValue;

    const auto usageCap = eSettings::sInstance->fRamMBCap;

    const longB enveUsedB(static_cast<long>(bytes_in_use_by_app));
    const intKB enveUsedKB(enveUsedB);
    if(usageCap.fValue > 0) {
        procFreeKB = intKB(usageCap) - enveUsedKB;
    } else {
        procFreeKB = HardwareInfo::sRamKB() - enveUsedKB;
    }

    const long freeInternal = physical_memory_used - bytes_in_use_by_app;
    intKB freeExternal(0);
    int found = 0;
    FILE * const meminfo = fopen("/proc/meminfo", "r");
    if(!meminfo) RuntimeThrow("Failed to open /proc/meminfo");
    while(fgets(sLine, sizeof(sLine), meminfo)) {
        int ramPartKB;
        if(sscanf(sLine, "MemFree: %d kB", &ramPartKB) == 1) {
//            qDebug() << "MemFree" << intMB(intKB(ramPartKB)).fValue;
        } else if(sscanf(sLine, "Cached: %d kB", &ramPartKB) == 1) {
//            qDebug() << "Cached" << intMB(intKB(ramPartKB)).fValue;
        } else if(sscanf(sLine, "Buffers: %d kB", &ramPartKB) == 1) {
//            qDebug() << "Buffers" << intMB(intKB(ramPartKB)).fValue;
        } else continue;
        freeExternal.fValue += ramPartKB;
        if(++found == 3) break;
    }
    fclose(meminfo);

    sysFreeKB = intKB(longB(freeInternal)) + freeExternal;

//    qDebug() << "free" << intMB(sysFreeKB).fValue;
//    qDebug() << "usage" << 100 - 100*sysFreeKB.fValue/HardwareInfo::sRamKB().fValue;
    if(found != 3) RuntimeThrow("Entries missing from /proc/meminfo");
    const long releaseBytes = 500L*1024L*1024L;
    if(freeInternal > releaseBytes) {
        MallocExtension::instance()->ReleaseToSystem(releaseBytes);
//        qDebug() << "released";
    }
}

void MemoryChecker::checkMemory() {
    intKB procFreeKB;
    intKB sysFreeKB;
    sGetFreeKB(procFreeKB, sysFreeKB);

    if(sysFreeKB < mLowFreeKB) {
        const intKB toFree = mLowFreeKB - sysFreeKB;
        if(sysFreeKB < mCriticalFreeKB) {
            emit handleMemoryState(CRITICAL_MEMORY_STATE, longB(toFree));
            mLastMemoryState = CRITICAL_MEMORY_STATE;
        } else if(sysFreeKB < mVeryLowFreeKB) {
            emit handleMemoryState(VERY_LOW_MEMORY_STATE, longB(toFree));
            mLastMemoryState = VERY_LOW_MEMORY_STATE;
        } else {
            emit handleMemoryState(LOW_MEMORY_STATE, longB(toFree));
            mLastMemoryState = LOW_MEMORY_STATE;
        }
    } else if(procFreeKB.fValue < 0) {
        emit handleMemoryState(LOW_MEMORY_STATE, longB(-procFreeKB));
        mLastMemoryState = LOW_MEMORY_STATE;
    } else if(mLastMemoryState != NORMAL_MEMORY_STATE) {
        emit handleMemoryState(NORMAL_MEMORY_STATE, longB(0));
        mLastMemoryState = NORMAL_MEMORY_STATE;
    }

    emit memoryCheckedKB(sysFreeKB, HardwareInfo::sRamKB());
}
