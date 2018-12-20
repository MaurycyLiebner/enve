#include "updatable.h"
#include "GUI/mainwindow.h"
#include "taskexecutor.h"

_ScheduledTask::~_ScheduledTask() {
    clear();
}

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

_Task::_Task() {}

void _Task::setCurrentTaskExecutor(TaskExecutor *taskExecutor) {
    mCurrentTaskExecutor = taskExecutor;
}

void _Task::beforeProcessingStarted() {
    mSelfRef = ref<_Task>();
    mBeingProcessed = true;
    mCurrentExecutionDependent = mNextExecutionDependent;
    mNextExecutionDependent.clear();
}

void _Task::finishedProcessing() {
    mFinished = true;
    mBeingProcessed = false;
    afterProcessingFinished();
    mSelfRef.reset();
}

void _Task::afterProcessingFinished() {
    mCurrentTaskExecutor = nullptr;
    tellDependentThatFinished();
}

bool _Task::isBeingProcessed() { return mBeingProcessed; }

void _Task::waitTillProcessed() {
    if(mCurrentTaskExecutor == nullptr) {
        return;
    }
    {
        QEventLoop loop;
        loop.connect(mCurrentTaskExecutor,
                     SIGNAL(finishedUpdating(int, _Task*)),
                     SLOT(quit()));
        loop.exec();
    }
}

bool _Task::readyToBeProcessed() {
    return nDependancies == 0 && !mBeingProcessed;
}

void _Task::clear() {
    mFinished = false;
    tellDependentThatFinished();
    mBeingProcessed = false;
    mSelfRef.reset();
    foreach(const stdptr<_Task>& dependent, mNextExecutionDependent) {
        if(dependent == nullptr) continue;
        dependent->decDependencies();
    }
    mNextExecutionDependent.clear();
}

void _Task::addDependent(_Task *updatable) {
    if(updatable == nullptr) return;
    if(!finished()) {
        if(listContainsSharedPtr(updatable, mNextExecutionDependent)) return;
        mNextExecutionDependent << updatable;
        updatable->incDependencies();
    }
}

bool _Task::finished() { return mFinished; }

void _Task::decDependencies() {
    nDependancies--;
}

void _Task::incDependencies() {
    nDependancies++;
}

void _Task::tellDependentThatFinished() {
    foreach(const stdptr<_Task>& dependent, mCurrentExecutionDependent) {
        if(dependent == nullptr) continue;
        dependent->decDependencies();
    }
    mCurrentExecutionDependent.clear();
}
