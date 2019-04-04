#include "memorychecker.h"
#include <sys/sysinfo.h>
#include <gperftools/tcmalloc.h>
#include <gperftools/malloc_extension.h>
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include "exceptions.h"

MemoryChecker *MemoryChecker::mInstance;

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
    mLowFreeRam = 800000000;
    mVeryLowFreeRam = 500000000;
}

void MemoryChecker::setCurrentMemoryState(const MemoryState &state) {
    if(state == mCurrentMemoryState) return;
    if(state == NORMAL_MEMORY_STATE) {
        mPgFltSamples.clear();
        mLastPgFlts = -1;
    }
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
        if(freeMem < mVeryLowFreeRam) {
            emit handleMemoryState(VERY_LOW_MEMORY_STATE, mLowFreeRam - freeMem);
        } else {
            emit handleMemoryState(LOW_MEMORY_STATE, mLowFreeRam - freeMem);
        }
    }

    const int freeMB = static_cast<int>(freeMem/1000);
    const int totalMB = static_cast<int>(mTotalRam/1000);

    emit memoryChecked(freeMB, totalMB);
}

int getMajorPageFaults() {
    FILE * const meminfo = fopen("/proc/vmstat", "r");
    if(meminfo) {
        char line[256];
        while(fgets(line, sizeof(line), meminfo)) {
            int pgFlts;
            if(sscanf(line, "pgmajfault %d", &pgFlts) == 1) {
                fclose(meminfo);
                return pgFlts;
            }
        }
        fclose(meminfo);
        RuntimeThrow("Failed to properly parse /proc/vmstat for page faults check");
    }
    RuntimeThrow("Failed to open /proc/vmstat for page faults check");
}

void MemoryChecker::checkMajorMemoryPageFault() {
    const bool firstSample = mLastPgFlts == -1;
    const int pgFlts = getMajorPageFaults();

    if(firstSample) {
        mLastPgFlts = pgFlts;
        return;
    }
    const int relPgFlt = pgFlts - mLastPgFlts;
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
            const auto freeMem = getFreeRam();
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
