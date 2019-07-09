#include "updatable.h"
#include "taskscheduler.h"
#include "taskexecutor.h"

bool Task::scheduleTask() {
    if(mState == SCHEDULED) return false;
    mState = SCHEDULED;
    scheduleTaskNow();
    return true;
}

void Task::aboutToProcess() {
    mState = PROCESSING;
    beforeProcessing();
}

void Task::finishedProcessing() {
    if(mState == CANCELED) return;
    mState = FINISHED;
    afterProcessing();
    if(unhandledException()) {
        gPrintExceptionCritical(handleException());
        cancelDependent();
    } else {
        tellDependentThatFinished();
    }
}

bool Task::readyToBeProcessed() {
    return mNDependancies == 0;
}

void Task::addDependent(Task * const updatable) {
    if(!updatable) return;
    if(mState != FINISHED) {
        if(mDependent.contains(updatable)) return;
        mDependent << updatable;
        updatable->incDependencies();
    }
}

void Task::addDependent(const Dependent &func) {
    if(mState == FINISHED) {
        if(func.fFinished) func.fFinished();
    } else if(mState == CANCELED) {
        if(func.fCanceled) func.fCanceled();
    } else mDependentF << func;
}

bool Task::finished() { return mState == FINISHED; }

void Task::decDependencies() {
    mNDependancies--;
}

void Task::incDependencies() {
    mNDependancies++;
}

void Task::tellDependentThatFinished() {
    for(const auto& dependent : mDependent) {
        if(dependent) dependent->decDependencies();
    }
    mDependent.clear();
    for(const auto& dependent : mDependentF) {
        if(dependent.fFinished) dependent.fFinished();
    }
    mDependentF.clear();
}

void Task::cancelDependent() {
    for(const auto& dependent : mDependent) {
        if(dependent) dependent->cancel();
    }
    mDependent.clear();
    for(const auto& dependent : mDependentF) {
        if(dependent.fCanceled) dependent.fCanceled();
    }
    mDependentF.clear();
}

void CPUTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleCPUTask(ref<Task>());
}

void HDDTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleHDDTask(ref<Task>());
}

void HDDTask::HDDPartFinished() {
    if(mController) emit mController->HDDPartFinished();
}
