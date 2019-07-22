#include "simpletask.h"

QList<SimpleTask*> SimpleTask::sTasks;

SimpleTask::SimpleTask(const Func& func) : mFunc(func) {}

SimpleTask* SimpleTask::sSchedule(const Func &func) {
    const auto task = new SimpleTask(func);
    sTasks << task;
    return task;
}

void SimpleTask::sProcessAll() {
    for(int i = 0; i < sTasks.count(); i++) {
        const auto& task = sTasks.at(i);
        task->process();
        delete task;
    }
    sTasks.clear();
}

SimpleTaskScheduler::SimpleTaskScheduler(const Func &func) :
    mFunc(func) {}

void SimpleTaskScheduler::schedule() {
    if(mScheduled) return;
    const auto task = SimpleTask::sSchedule(mFunc);
    connect(task, &SimpleTask::destroyed, this, [this]() { mScheduled = false; });
    mScheduled = true;
}
