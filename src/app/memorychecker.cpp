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
    mLowFreeKB = totRam*25/100;
    mVeryLowFreeKB = totRam*20/100;
    mCriticalFreeKB = totRam*15/100;
}

char MemoryChecker::sLine[256];

void MemoryChecker::sGetFreeKB(intKB& procFreeKB, intKB& sysFreeKB) {
//    qDebug() << "";
    size_t allocated_bytes;
    size_t pageheap_unmapped_bytes;
    size_t pageheap_free_bytes;
    MallocExtension::instance()->GetAllocatedAndUnmapped(
                &allocated_bytes,
                &pageheap_unmapped_bytes,
                &pageheap_free_bytes);

//    qDebug() << "allocated" << intMB(longB(allocated_bytes)).fValue;
//    qDebug() << "unmapped" << intMB(longB(pageheap_unmapped_bytes)).fValue;
//    qDebug() << "free" << intMB(longB(pageheap_free_bytes)).fValue;
    const auto usageCap = eSettings::sInstance->fRamMBCap;

    if(usageCap.fValue > 0) {
        const longB enveUsedB(static_cast<long>(allocated_bytes));
        procFreeKB = intKB(usageCap) - intKB(enveUsedB);
    } else procFreeKB = HardwareInfo::sRamKB();

    FILE * const meminfo = fopen("/proc/meminfo", "r");
    if(!meminfo) RuntimeThrow("Failed to open /proc/meminfo");
    const long freeAndUnmapped = static_cast<long>(pageheap_unmapped_bytes +
                                                   pageheap_free_bytes);
    sysFreeKB = intKB(longB(freeAndUnmapped));
    int found = 0;
    while(fgets(sLine, sizeof(sLine), meminfo)) {
        int ramPartKB;
        if(sscanf(sLine, "MemFree: %d kB", &ramPartKB) == 1) {
//            qDebug() << "MemFree" << intMB(intKB(ramPartKB)).fValue;
            sysFreeKB.fValue += ramPartKB;
            found++;
        } else continue;

        if(found == 1) break;
    }
//    qDebug() << "total" << intMB(sysFreeKB).fValue;
//    qDebug() << "usage" << 100 - 100*sysFreeKB.fValue/HardwareInfo::sRamKB().fValue;
    fclose(meminfo);
    if(found != 1) RuntimeThrow("Entries missing from /proc/meminfo");
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
