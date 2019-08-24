// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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
