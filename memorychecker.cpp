#include "memorychecker.h"
#include <sys/sysinfo.h>
#include <gperftools/tcmalloc.h>
#include <gperftools/malloc_extension.h>
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>

MemoryChecker *MemoryChecker::mInstance;

MemoryChecker::MemoryChecker(QObject *parent) : QObject(parent) {
    mInstance = this;
    struct sysinfo info;
    if(sysinfo(&info) == 0) {
        unsigned long long memUnit = info.mem_unit;
        unsigned long long totalRam = info.totalram*memUnit;
//        mFreeRam = info.freeram*info.mem_unit +
//                   //info.freeswap*info.mem_unit +
//                   info.bufferram*info.mem_unit;
        mMinFreeRam = totalRam*20/100;
    } else { // ??
    }
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()),
            this, SLOT(checkMemory()) );
    mTimer->start(500);
}

unsigned long long getFreeRam() {
    unsigned long long unmapped = 0ULL;
    char buffer[1000];
    MallocExtension::instance()->GetStats(buffer, 1000);

    QString allText = QString::fromUtf8(buffer);
    QStringList lines = allText.split(QRegExp("\n|\r\n|\r"));
    if(lines.count() > 9) {
        QString extract = lines.at(9);
        extract = extract.split('(').first();
        extract = extract.split('+').last();
        extract = extract.trimmed();
        unmapped = extract.toULongLong();
    } else {
        Q_FOREACH(const QString &line, lines) {
            if(line.contains("Bytes released to OS (aka unmapped)")) {
                QString extract = line.split('(').first();
                extract = extract.split('+').last();
                extract = extract.trimmed();
                unmapped = extract.toULongLong();
                break;
            }
        }
    }

    FILE *meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL) return 0;

    char line[256];
    unsigned long long ramULL = 0;
    int found = 0;
    while(fgets(line, sizeof(line), meminfo)) {
        int ram;
        if(sscanf(line, "MemFree: %d kB", &ram) == 1) {
            ramULL += (unsigned long long)ram*1000ULL;
            found++;
        } else if(sscanf(line, "Cached: %d kB", &ram) == 1) {
            ramULL += (unsigned long long)ram*1000ULL;
            found++;
        } else if(sscanf(line, "Buffers: %d kB", &ram) == 1) {
            ramULL += (unsigned long long)ram*1000ULL;
            found++;
        }

        if(found >= 3) {
            fclose(meminfo);
            return ramULL + unmapped;
        }
    }

    // If we got here, then we couldn't find the proper line in the meminfo file:
    // do something appropriate like return an error code, throw an exception, etc.
    fclose(meminfo);
    return ramULL + unmapped;
}

void MemoryChecker::checkMemory() {
    unsigned long long freeMem = getFreeRam();
    if(freeMem < mMinFreeRam) {
        emit outOfMemory(mMinFreeRam - freeMem);
    }
//    struct sysinfo info;
//    if(sysinfo(&info) == 0) {
//        mFreeRam = info.freeram*info.mem_unit +
//                   //info.freeswap*info.mem_unit +
//                   info.bufferram*info.mem_unit;
//        if(mFreeRam < mMinFreeRam) {
//            emit outOfMemory(mMinFreeRam);
//        }
//    }
//    if(mTotalRam < mLeaveUnused + mUsedRam) {
//        emit outOfMemory(mUsedRam + mLeaveUnused - mTotalRam);
//    }
}
