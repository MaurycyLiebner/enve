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

    mLowFreeKB = HardwareInfo::sRamKB();
    mLowFreeKB.fValue *= 20; mLowFreeKB.fValue /= 100;
    mVeryLowFreeKB = HardwareInfo::sRamKB();
    mVeryLowFreeKB.fValue *= 15; mVeryLowFreeKB.fValue /= 100;
}

char MemoryChecker::sLine[256];

void MemoryChecker::sGetFreeKB(intKB& procFreeKB, intKB& sysFreeKB) {
    size_t allocatedB = 0;
    size_t unmappedB = 0;
    MallocExtension::instance()->GetAllocatedAndUnmapped(&allocatedB, &unmappedB);
    const auto usageCap = eSettings::sInstance->fRamMBCap;
    procFreeKB = HardwareInfo::sRamKB();
    if(usageCap.fValue > 0) {
        const longB enveUsedB(static_cast<long>(allocatedB));
        procFreeKB = intKB(usageCap) - intKB(enveUsedB);
        qDebug() << intMB(usageCap).fValue << intMB(enveUsedB).fValue;
    }

    FILE * const meminfo = fopen("/proc/meminfo", "r");
    if(meminfo) {
        intKB ramKB(0);
        int found = 0;
        while(fgets(sLine, sizeof(sLine), meminfo)) {
            int ramPartKB;
            if(sscanf(sLine, "MemFree: %d kB", &ramPartKB) == 1) {
                ramKB.fValue += ramPartKB;
                found++;
            } else if(sscanf(sLine, "Cached: %d kB", &ramPartKB) == 1) {
                ramKB.fValue += ramPartKB;
                found++;
            } else if(sscanf(sLine, "Buffers: %d kB", &ramPartKB) == 1) {
                ramKB.fValue += ramPartKB;
                found++;
            }

            if(found >= 3) {
                fclose(meminfo);
                const intKB unmappedKB = intKB(longB(static_cast<long>(unmappedB)));
                sysFreeKB = ramKB + unmappedKB;
                return;
            }
        }
        fclose(meminfo);
        RuntimeThrow("Entries missing from /proc/meminfo");
    }
    RuntimeThrow("Failed to open /proc/meminfo");
}

void MemoryChecker::checkMemory() {
    intKB procFreeKB;
    intKB sysFreeKB;
    sGetFreeKB(procFreeKB, sysFreeKB);

    if(sysFreeKB < mLowFreeKB) {
        const intKB toFree = mLowFreeKB - sysFreeKB;
        if(sysFreeKB < mVeryLowFreeKB) {
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
