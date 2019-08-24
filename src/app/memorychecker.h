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

#ifndef MEMORYCHECKER_H
#define MEMORYCHECKER_H

#include <QObject>
#include <QTimer>

enum MemoryState {
    NORMAL_MEMORY_STATE,
    LOW_MEMORY_STATE = 5,
    VERY_LOW_MEMORY_STATE = 15,
    CRITICAL_MEMORY_STATE = 30
};

class MemoryChecker : public QObject {
    Q_OBJECT
public:
    explicit MemoryChecker(QObject * const parent = nullptr);
    static MemoryChecker *getInstance() { return mInstance; }

    void checkMemory();
private:
    static long sGetFreeBytes();
    static char sLine[256];

    MemoryState mLastMemoryState = NORMAL_MEMORY_STATE;

    long mLowFreeBytes = 0;
    long mVeryLowFreeBytes = 0;

    static MemoryChecker *mInstance;
signals:
    void memoryCheckedKB(int, int);
    void handleMemoryState(MemoryState, long bytesToFree);
};

#endif // MEMORYCHECKER_H
