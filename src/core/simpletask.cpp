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

#include "simpletask.h"

#include <QPointer>

QList<SimpleTask*> SimpleTask::sTasks;

SimpleTask::SimpleTask(const Func& func) : mFunc(func) {}

SimpleTask *SimpleTask::sScheduleContexted(
        const QPointer<QObject> &ctxt,
        const Func &func) {
    return sSchedule([func, ctxt]() { if(ctxt) func(); });
}

SimpleTask* SimpleTask::sSchedule(const Func &func) {
    const auto task = new SimpleTask(func);
    sTasks << task;
    return task;
}

void SimpleTask::sProcessAll() {
    for(int i = 0; i < sTasks.count(); i++) {
        const auto task = sTasks.at(i);
        task->process();
        delete task;
    }
    sTasks.clear();
}

SimpleTaskScheduler::SimpleTaskScheduler(const Func &func) :
    mFunc(func) {}

void SimpleTaskScheduler::schedule() {
    if(mScheduled) return;
    const auto task = SimpleTask::sScheduleContexted(this, mFunc);
    connect(task, &SimpleTask::destroyed, this, [this]() { mScheduled = false; });
    mScheduled = true;
}
