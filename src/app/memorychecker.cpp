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

    mVeryLowFreeBytes = 15*HardwareInfo::sRamBytes()/100;
    mLowFreeBytes = 20*HardwareInfo::sRamBytes()/100;
}

char MemoryChecker::sLine[256];

long MemoryChecker::sGetFreeBytes() {
    size_t unmappedB;
    MallocExtension::instance()->GetNumericProperty(
                "tcmalloc.pageheap_unmapped_bytes", &unmappedB);

    FILE * const meminfo = fopen("/proc/meminfo", "r");
    if(meminfo) {
        ulong ramKB = 0;
        int found = 0;
        while(fgets(sLine, sizeof(sLine), meminfo)) {
            uint ramPartKB;
            if(sscanf(sLine, "MemFree: %d kB", &ramPartKB) == 1) {
                ramKB += ramPartKB;
                found++;
            } else if(sscanf(sLine, "Cached: %d kB", &ramPartKB) == 1) {
                ramKB += ramPartKB;
                found++;
            } else if(sscanf(sLine, "Buffers: %d kB", &ramPartKB) == 1) {
                ramKB += ramPartKB;
                found++;
            }

            if(found >= 3) {
                fclose(meminfo);
                return static_cast<long>(ramKB)*1000 +
                       static_cast<long>(unmappedB);
            }
        }
        fclose(meminfo);
        RuntimeThrow("Entries missing from /proc/meminfo");
    }
    RuntimeThrow("Failed to open /proc/meminfo");
}

void MemoryChecker::checkMemory() {
    const long freeBytes = sGetFreeBytes();

    if(freeBytes < mLowFreeBytes) {
        const long toFree = mLowFreeBytes - freeBytes;
        if(freeBytes < mVeryLowFreeBytes) {
            emit handleMemoryState(VERY_LOW_MEMORY_STATE, toFree);
            mLastMemoryState = VERY_LOW_MEMORY_STATE;
        } else {
            emit handleMemoryState(LOW_MEMORY_STATE, toFree);
            mLastMemoryState = LOW_MEMORY_STATE;
        }
    } else if(mLastMemoryState != NORMAL_MEMORY_STATE) {
        emit handleMemoryState(NORMAL_MEMORY_STATE, 0);
        mLastMemoryState = NORMAL_MEMORY_STATE;
    }

    emit memoryCheckedKB(static_cast<int>(freeBytes/1000),
                         HardwareInfo::sRamKB());
}
