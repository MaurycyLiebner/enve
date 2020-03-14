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

#include "taskexecutor.h"

void TaskExecutor::processTask(const stdsptr<eTask>& task) {
    try {
        task->process();
    } catch(...) {
        task->setException(std::current_exception());
    }

    emit finishedTask(task);
}

QAtomicList<stdsptr<eTask>> CpuTaskExecutor::sReadyToProcess;
QAtomicInt CpuTaskExecutor::sProcessingCount;

void CpuTaskExecutor::sAddReadyToProcess(const QList<stdsptr<eTask>>& ready) {
    sReadyToProcess.appendAndNotifyAll(ready);
}

int CpuTaskExecutor::sUsedCount() {
    return sProcessingCount;
}

void CpuTaskExecutor::start() {
    while(true) {
        stdsptr<eTask> task;
        sReadyToProcess.waitTakeFirst(task);
        sProcessingCount++;
        processTask(task);
        sProcessingCount--;
    }
}
