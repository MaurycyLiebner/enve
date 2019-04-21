#include "updatable.h"
#include "GUI/mainwindow.h"
#include "taskexecutor.h"

bool Task::scheduleTask() {
    if(mState == SCHEDULED) return false;

    mState = SCHEDULED;
    scheduleTaskNow();
    return true;
}

void Task::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleCPUTask(ref<Task>());
}

void Task::beforeProcessingStarted() {
    mState = PROCESSING;
    mCurrentExecutionDependent = mNextExecutionDependent;
    mNextExecutionDependent.clear();
}

void Task::finishedProcessing() {
    mState = FINISHED;
    tellDependentThatFinished();
    afterProcessingFinished();
    if(unhandledException()) {
        gPrintExceptionCritical(handleException());
    }
}

bool Task::readyToBeProcessed() {
    return mNDependancies == 0;
}

void Task::clear() {
    mState = CREATED;
    tellDependentThatFinished();
    tellNextDependentThatFinished();
}

void Task::addDependent(Task * const updatable) {
    if(!updatable) return;
    if(mState != FINISHED) {
        if(mNextExecutionDependent.contains(updatable)) return;
        mNextExecutionDependent << updatable;
        updatable->incDependencies();
    }
}

bool Task::finished() { return mState == FINISHED; }

void Task::decDependencies() {
    mNDependancies--;
}

void Task::incDependencies() {
    mNDependancies++;
}

void Task::tellDependentThatFinished() {
    for(const auto& dependent : mCurrentExecutionDependent) {
        if(dependent) dependent->decDependencies();
    }
    mCurrentExecutionDependent.clear();
}

void Task::tellNextDependentThatFinished() {
    for(const auto& dependent : mNextExecutionDependent) {
        if(dependent) dependent->decDependencies();
    }
    mNextExecutionDependent.clear();
}

void _HDDTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleHDDTask(ref<Task>());
}
