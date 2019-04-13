#include "updatable.h"
#include "GUI/mainwindow.h"
#include "taskexecutor.h"

_Task::State _ScheduledTask::SCHEDULED =
        static_cast<_Task::State>(2);
_Task::State _ScheduledTask::QUED =
        static_cast<_Task::State>(3);

void _ScheduledTask::taskQued() {
    mState = QUED;
}

bool _ScheduledTask::scheduleTask() {
    if(!canSchedule() || mState == SCHEDULED) return false;

    mState = SCHEDULED;
    scheduleTaskNow();
    return true;
}

void _ScheduledTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleCPUTask(ref<_ScheduledTask>());
}

void _HDDTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleHDDTask(ref<_ScheduledTask>());
}

_Task::_Task() {}

void _Task::beforeProcessingStarted() {
    Q_ASSERT(mCurrentExecutionDependent.isEmpty());
    mState = PROCESSING;
    mCurrentExecutionDependent = mNextExecutionDependent;
    mNextExecutionDependent.clear();
}

void _Task::finishedProcessing() {
    mState = FINISHED;
    tellDependentThatFinished();
    afterProcessingFinished();
}

bool _Task::readyToBeProcessed() {
    return mNDependancies == 0;
}

void _Task::clear() {
    mState = CREATED;
    tellDependentThatFinished();
    tellNextDependentThatFinished();
}

void _Task::addDependent(_Task * const updatable) {
    if(!updatable) return;
    if(mState != FINISHED) {
        if(mNextExecutionDependent.contains(updatable)) return;
        mNextExecutionDependent << updatable;
        updatable->incDependencies();
    }
}

bool _Task::finished() { return mState == FINISHED; }

void _Task::decDependencies() {
    mNDependancies--;
}

void _Task::incDependencies() {
    mNDependancies++;
}

void _Task::tellDependentThatFinished() {
    for(const auto& dependent : mCurrentExecutionDependent) {
        if(dependent) dependent->decDependencies();
    }
    mCurrentExecutionDependent.clear();
}

void _Task::tellNextDependentThatFinished() {
    for(const auto& dependent : mNextExecutionDependent) {
        if(dependent) dependent->decDependencies();
    }
    mNextExecutionDependent.clear();
}
