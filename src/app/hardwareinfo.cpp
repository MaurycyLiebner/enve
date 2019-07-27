#include "hardwareinfo.h"

#include <QThread>

#include "exceptions.h"

int HardwareInfo::mCpuThreads = -1;

long HardwareInfo::mRamBytes = -1;
int HardwareInfo::mRamKB = -1;

long getTotalRamBytes() {
    FILE * const meminfo = fopen("/proc/meminfo", "r");
    if(meminfo) {
        char line[256];

        while(fgets(line, sizeof(line), meminfo)) {
            int ram;
            if(sscanf(line, "MemTotal: %d kB", &ram) == 1) {
                fclose(meminfo);
                return static_cast<long>(ram)*1000;
            }
        }
        fclose(meminfo);
        RuntimeThrow("'MemTotal' missing from /proc/meminfo");
    }
    RuntimeThrow("Failed to open /proc/meminfo");
}

void HardwareInfo::sUpdateInfo() {
    mCpuThreads = QThread::idealThreadCount();
    mRamBytes = getTotalRamBytes();
    mRamKB = static_cast<int>(mRamBytes/1000);
}
