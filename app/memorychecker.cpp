#include "memorychecker.h"
#include <sys/sysinfo.h>
#include <gperftools/tcmalloc.h>
#include <gperftools/malloc_extension.h>
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>

MemoryChecker *MemoryChecker::mInstance;

unsigned long long getTotalRam() {
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if(!meminfo) return 0;

    char line[256];

    while(fgets(line, sizeof(line), meminfo)) {
        int ram;
        if(sscanf(line, "MemTotal: %d kB", &ram) == 1) {
            fclose(meminfo);
            return (unsigned long long)ram*1000ULL;
        }
    }

    // If we got here, then we couldn't find the proper line in the meminfo file:
    // do something appropriate like return an error code, throw an exception, etc.
    fclose(meminfo);
    return 0;
}

MemoryChecker::MemoryChecker(QObject *parent) : QObject(parent) {
    mInstance = this;

    mTotalRam = getTotalRam();
    mLowFreeRam = 800000000;
    mVeryLowFreeRam = 500000000;
}

void MemoryChecker::setCurrentMemoryState(const MemoryState &state) {
    if(state == mCurrentMemoryState) return;
//    qDebug() << "set state: " << state;
    //bool worsend = state > mCurrentMemoryState;
    if(state == NORMAL_MEMORY_STATE) {
        mPgFltSamples.clear();
        mLastPgFlts = -1;
    }
    mCurrentMemoryState = state;
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

    FILE *meminfo = fopen("/proc/meminfo", "r");
    if(!meminfo) return 0;

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
    //qDebug() << "freemem: " << freeMem;

    if(freeMem < mLowFreeRam) {
        if(freeMem < mVeryLowFreeRam) {
            emit handleMemoryState(VERY_LOW_MEMORY_STATE, mLowFreeRam - freeMem);
        } else {
            emit handleMemoryState(LOW_MEMORY_STATE, mLowFreeRam - freeMem);
        }
        //setCurrentMemoryState(LOW_MEMORY_STATE);
    }

    emit memoryChecked((int)(freeMem/1000ULL), (int)(mTotalRam/1000ULL));
}

unsigned long long getMajorPageFaults() {
    FILE *meminfo = fopen("/proc/vmstat", "r");
    if(!meminfo) return 0;

    char line[256];
    while(fgets(line, sizeof(line), meminfo)) {
        int pgFlts;
        if(sscanf(line, "pgmajfault %d", &pgFlts) == 1) {
            fclose(meminfo);
            return pgFlts;
        }
    }

    // If we got here, then we couldn't find the proper line in the meminfo file:
    // do something appropriate like return an error code, throw an exception, etc.
    fclose(meminfo);
    return 0;
}

void MemoryChecker::checkMajorMemoryPageFault() {
    bool firstSample = mLastPgFlts == -1;
    int pgFlts = getMajorPageFaults();

    if(firstSample) {
        mLastPgFlts = pgFlts;
        return;
    }
    int relPgFlt = pgFlts - mLastPgFlts;
    mLastPgFlts = pgFlts;
    if(mPgFltSamples.count() == 3) {
        mPgFltSamples.removeFirst();
    }
    mPgFltSamples << relPgFlt;
    int avgPgFlts = 0;
    for(const int &sample : mPgFltSamples) {
        avgPgFlts += sample;
    }
    avgPgFlts = avgPgFlts/mPgFltSamples.count();
    //qDebug() << "avg pgflts: " << avgPgFlts;
    if(avgPgFlts > mCurrentMemoryState) {
        if(mCurrentMemoryState == LOW_MEMORY_STATE &&
                avgPgFlts > VERY_LOW_MEMORY_STATE) {
            setCurrentMemoryState(VERY_LOW_MEMORY_STATE);
        } else if(mCurrentMemoryState == VERY_LOW_MEMORY_STATE &&
                  avgPgFlts > CRITICAL_MEMORY_STATE) {
            setCurrentMemoryState(CRITICAL_MEMORY_STATE);
        }
    } else {
        if(mCurrentMemoryState == LOW_MEMORY_STATE) {
            unsigned long long freeMem = getFreeRam();
            //qDebug() << "freemem: " << freeMem;

            if(freeMem > mLowFreeRam) {
                setCurrentMemoryState(NORMAL_MEMORY_STATE);
                return;
            }
        } else if(mCurrentMemoryState == VERY_LOW_MEMORY_STATE) {
            setCurrentMemoryState(LOW_MEMORY_STATE);
        } else if(mCurrentMemoryState == CRITICAL_MEMORY_STATE) {
            setCurrentMemoryState(VERY_LOW_MEMORY_STATE);
        }
    }

    emit handleMemoryState(mCurrentMemoryState, mLowFreeRam);
}
