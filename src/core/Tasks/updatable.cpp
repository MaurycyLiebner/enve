#include "Tasks/updatable.h"
#include "Tasks/taskscheduler.h"
#include "taskexecutor.h"

bool eTask::scheduleTask() {
    if(mState == eTaskState::scheduled) return false;
    mState = eTaskState::scheduled;
    scheduleTaskNow();
    return true;
}

void eTask::aboutToProcess(const Hardware hw) {
    mState = eTaskState::processing;
    beforeProcessing(hw);
}

void eTask::finishedProcessing() {
    if(mState == eTaskState::canceled || mState == eTaskState::failed) return;
    mState = eTaskState::finished;
    afterProcessing();
    if(unhandledException()) {
        gPrintExceptionCritical(handleException());
        cancelDependent();
    } else {
        tellDependentThatFinished();
    }
}

bool eTask::readyToBeProcessed() {
    return mNDependancies == 0;
}

void eTask::addDependent(eTask * const updatable) {
    if(!updatable) return;
    if(mState != eTaskState::finished) {
        if(mDependent.contains(updatable)) return;
        mDependent << updatable;
        updatable->incDependencies();
    }
}

void eTask::addDependent(const Dependent &func) {
    if(mState == eTaskState::finished) {
        if(func.fFinished) func.fFinished();
    } else if(mState == eTaskState::canceled) {
        if(func.fCanceled) func.fCanceled();
    } else mDependentF << func;
}

bool eTask::finished() { return mState == eTaskState::finished; }

void eTask::decDependencies() {
    mNDependancies--;
}

void eTask::incDependencies() {
    mNDependancies++;
}

void eTask::tellDependentThatFinished() {
    for(const auto& dependent : mDependent) {
        if(dependent) dependent->decDependencies();
    }
    mDependent.clear();
    for(const auto& dependent : mDependentF) {
        if(dependent.fFinished) dependent.fFinished();
    }
    mDependentF.clear();
}

void eTask::cancelDependent() {
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
    TaskScheduler::sGetInstance()->scheduleCPUTask(ref<eTask>());
}

void HDDTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleHDDTask(ref<eTask>());
}

void HDDTask::HDDPartFinished() {
    if(mController) emit mController->HDDPartFinished();
}
