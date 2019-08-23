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
    mState = eTaskState::finished;
    if(mCancel) {
        mCancel = false;
        cancel();
    } else if(unhandledException()) {
        gPrintExceptionCritical(handleException());
        cancel();
    } else {
        afterProcessing();
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

void eCpuTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleCpuTask(ref<eTask>());
}

void eHddTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleHddTask(ref<eTask>());
}

void eHddTask::HddPartFinished() {
    if(mController) emit mController->HddPartFinished();
}
