#include "memorychecker.h"
#include <sys/sysinfo.h>
#include <gperftools/tcmalloc.h>
#include <gperftools/malloc_extension.h>
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include "exceptions.h"
#include <fstream>
#include <unistd.h>
MemoryChecker *MemoryChecker::mInstance;

void getProcessUsedRam(double& vm_usage, double& resident_set) {
    vm_usage = 0;
    resident_set = 0;

    // the two fields we want
    unsigned long vsize;
    long rss;
    {
        std::string ignore;
        std::ifstream ifs("/proc/self/stat", std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> vsize >> rss;
    }

    // in case x86-64 is configured to use 2MB pages
    const long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;
    vm_usage = vsize / 1024.;
    resident_set = rss * page_size_kb;
}

unsigned long long getTotalRam() {
    FILE * const meminfo = fopen("/proc/meminfo", "r");
    if(meminfo) {
        char line[256];

        while(fgets(line, sizeof(line), meminfo)) {
            int ram;
            if(sscanf(line, "MemTotal: %d kB", &ram) == 1) {
                fclose(meminfo);
                return static_cast<unsigned long long>(ram)*1000;
            }
        }
        fclose(meminfo);
        RuntimeThrow("Failed to properly parse /proc/meminfo for total memory check");
    }
    RuntimeThrow("Failed to open /proc/meminfo for total memory check");
}

MemoryChecker::MemoryChecker(QObject * const parent) : QObject(parent) {
    mInstance = this;

    mTotalRam = getTotalRam();
    mVeryLowFreeRam = 15*mTotalRam/100;
    mLowFreeRam = 20*mTotalRam/100;
}

void MemoryChecker::setCurrentMemoryState(const MemoryState &state) {
    if(state == mCurrentMemoryState) return;
    mCurrentMemoryState = state;
}

unsigned long long getFreeRam() {
    unsigned long long unmapped = 0;
    char buffer[1000];
    MallocExtension::instance()->GetStats(buffer, 1000);

    const QString allText = QString::fromUtf8(buffer);
    const QStringList lines = allText.split(QRegExp("\n|\r\n|\r"));
    if(lines.count() > 9) {
        QString extract = lines.at(9);
        extract = extract.split('(').first();
        extract = extract.split('+').last();
        extract = extract.trimmed();
        unmapped = extract.toULongLong();
    } else {
        for(const QString &line : lines) {
            if(line.contains("Bytes released to OS (aka unmapped)")) {
                QString extract = line.split('(').first();
                extract = extract.split('+').last();
                extract = extract.trimmed();
                unmapped = extract.toULongLong();
                break;
            }
        }
    }

    FILE * const meminfo = fopen("/proc/meminfo", "r");
    if(meminfo) {
        char line[256];
        unsigned long long ramULL = 0;
        int found = 0;
        while(fgets(line, sizeof(line), meminfo)) {
            int ram;
            if(sscanf(line, "MemFree: %d kB", &ram) == 1) {
                ramULL += static_cast<unsigned long long>(ram)*1000;
                found++;
            } else if(sscanf(line, "Cached: %d kB", &ram) == 1) {
                ramULL += static_cast<unsigned long long>(ram)*1000;
                found++;
            } else if(sscanf(line, "Buffers: %d kB", &ram) == 1) {
                ramULL += static_cast<unsigned long long>(ram)*1000;
                found++;
            }

            if(found >= 3) {
                fclose(meminfo);
                return ramULL + unmapped;
            }
        }
        fclose(meminfo);
        RuntimeThrow("Failed to properly parse /proc/meminfo for free memory check");
    }
    RuntimeThrow("Failed to open /proc/meminfo for free memory check");
}

void MemoryChecker::checkMemory() {
    const auto freeMem = getFreeRam();

    if(freeMem < mLowFreeRam) {
        const auto toFree = mLowFreeRam - freeMem;
        if(freeMem < mVeryLowFreeRam) {
            emit handleMemoryState(VERY_LOW_MEMORY_STATE, toFree);
        } else {
            emit handleMemoryState(LOW_MEMORY_STATE, toFree);
        }
    }

    const int freeMB = static_cast<int>(freeMem/1000);
    const int totalMB = static_cast<int>(mTotalRam/1000);

    emit memoryChecked(freeMB, totalMB);
}
