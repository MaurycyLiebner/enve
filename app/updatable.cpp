#include "updatable.h"
#include "GUI/mainwindow.h"
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
    mState = FINISHED;
    tellDependentThatFinished();
    afterProcessing();
    if(unhandledException()) {
        gPrintExceptionCritical(handleException());
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
}

void CPUTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleCPUTask(ref<Task>());
}

void HDDTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleHDDTask(ref<Task>());
}
