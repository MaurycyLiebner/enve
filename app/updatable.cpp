#include "updatable.h"
#include "GUI/mainwindow.h"
#include "taskexecutor.h"

void _ScheduledTask::beforeProcessingStarted() {
    _Task::beforeProcessingStarted();
    mTaskQued = false;
}

void _ScheduledTask::taskQued() {
    mTaskQued = true;
    mTaskScheduled = false;
}

bool _ScheduledTask::scheduleTask() {
    if(!shouldUpdate() || mTaskScheduled) return false;

    mFinished = false;
    mTaskScheduled = true;
    scheduleTaskNow();
    return true;
}

void _ScheduledTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleCPUTask(ref<_ScheduledTask>());
}

void _ScheduledTask::clear() {
    mTaskScheduled = false;
    mTaskQued = false;
    _Task::clear();
}

void _HDDTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleHDDTask(ref<_ScheduledTask>());
}

_Task::_Task() {}

void _Task::setCurrentTaskExecutor(TaskExecutor *taskExecutor) {
    mCurrentTaskExecutor = taskExecutor;
}

void _Task::beforeProcessingStarted() {
    mSelfRef = ref<_Task>();
    mBeingProcessed = true;
    Q_ASSERT(mCurrentExecutionDependent.isEmpty());
    mCurrentExecutionDependent = mNextExecutionDependent;
    mNextExecutionDependent.clear();
}

void _Task::finishedProcessing() {
    mFinished = true;
    mBeingProcessed = false;
    mCurrentTaskExecutor = nullptr;
    tellDependentThatFinished();
    afterProcessingFinished();
    mSelfRef.reset();
}

void _Task::afterProcessingFinished() {

}

bool _Task::isBeingProcessed() { return mBeingProcessed; }

void _Task::waitTillProcessed() {
    if(!mCurrentTaskExecutor) return;
    {
        QEventLoop loop;
        QObject::connect(
                    mCurrentTaskExecutor, &TaskExecutor::finishedUpdating,
                    &loop, &QEventLoop::quit);
        loop.exec();
    }
}

bool _Task::readyToBeProcessed() {
    return mNDependancies == 0 && !mBeingProcessed;
}

void _Task::clear() {
    mFinished = false;
    tellDependentThatFinished();
    tellNextDependentThatFinished();
    mBeingProcessed = false;
    mSelfRef.reset();
}

void _Task::addDependent(_Task * const updatable) {
    if(!updatable) return;
    if(!finished()) {
        if(listContainsSharedPtr(updatable, mNextExecutionDependent)) return;
        mNextExecutionDependent << updatable;
        updatable->incDependencies();
    }
}

bool _Task::finished() { return mFinished; }

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
