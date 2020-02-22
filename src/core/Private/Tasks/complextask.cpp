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

#include "complextask.h"

ComplexTask::ComplexTask(const QString &name) : mName(name) {}

void ComplexTask::addPlannedTasks(const int tasks) {
    mPlannedCount += tasks;
}

void ComplexTask::removePlannedTasks(const int tasks) {
    mPlannedCount -= tasks;
    finishedEmitters();
}

void ComplexTask::addPlannedTask(const stdsptr<eTask> &task) {
    mPlannedCount--;
    addTask(task);
}

void ComplexTask::addTask(const stdsptr<eTask> &task) {
    const qptr<ComplexTask> ptr = this;
    const auto finishedInc = [ptr]() {
        if(ptr) ptr->incFinished();
    };
    mTasks << task;
    task->addDependent({finishedInc, finishedInc});
}

void ComplexTask::cancel() {
    if(mDone || !mStarted) return;
    mDone = true;
    for(const auto& task : mTasks) {
        const eTaskState state = task->getState();
        if(state <= eTaskState::processing) task->cancel();
    }
    emit canceled();
}

int ComplexTask::count() const { return mTasks.count() + mPlannedCount; }

const QString &ComplexTask::name() const { return mName; }

int ComplexTask::finishedCount() const { return mFinishedCount; }

void ComplexTask::finishedEmitters() {
    if(mDone || !mStarted) return;
    emit finished(mFinishedCount);
    if(mFinishedCount >= count()) {
        emit finishedAll();
        mDone = true;
    }
}

void ComplexTask::incFinished() {
    mFinishedCount++;
    finishedEmitters();
}
