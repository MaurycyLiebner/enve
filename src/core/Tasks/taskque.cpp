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

#include "taskque.h"

TaskQue::TaskQue() {}

TaskQue::~TaskQue() {
    for(const auto& cpuTask : mQued) cpuTask->cancel();
    for(const auto& gcpuTask : mGpuPreffered) gcpuTask->cancel();
    for(const auto& gpuTask : mGpuOnly) gpuTask->cancel();
}

int TaskQue::countQued() const {
    return mQued.count() +
           mGpuPreffered.count() +
           mGpuOnly.count();
}

bool TaskQue::allDone() const { return countQued() == 0; }

void TaskQue::addTask(const stdsptr<eTask> &task) {
    const auto hwSupport = task->hardwareSupport();
    if(hwSupport == HardwareSupport::gpuOnly) mGpuOnly << task;
    else if(hwSupport == HardwareSupport::gpuPreffered) mGpuPreffered << task;
    else mQued << task;
}

stdsptr<eTask> TaskQue::takeQuedForCpuProcessing() {
    for(int i = 0; i < mQued.count(); i++) {
        const auto& task = mQued.at(i);
        if(task->readyToBeProcessed())
            return mQued.takeAt(i);
    }
    for(int i = 0; i < mGpuPreffered.count(); i++) {
        const auto& task = mGpuPreffered.at(i);
        if(task->readyToBeProcessed())
            return mGpuPreffered.takeAt(i);
    }
    return nullptr;
}

stdsptr<eTask> TaskQue::takeQuedForGpuProcessing() {
    for(int i = 0; i < mGpuOnly.count(); i++) {
        const auto& task = mGpuOnly.at(i);
        if(task->readyToBeProcessed())
            return mGpuOnly.takeAt(i);
    }
    for(int i = 0; i < mGpuPreffered.count(); i++) {
        const auto& task = mGpuPreffered.at(i);
        if(task->readyToBeProcessed())
            return mGpuPreffered.takeAt(i);
    }
    for(int i = 0; i < mQued.count(); i++) {
        const auto& task = mQued.at(i);
        if(!task->readyToBeProcessed()) continue;
        if(task->hardwareSupport() == HardwareSupport::cpuOnly) continue;
        return mQued.takeAt(i);
    }
    return nullptr;
}
