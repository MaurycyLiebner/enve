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

void Task::cancelDependent() {
    for(const auto& dependent : mDependent) {
        if(dependent) dependent->cancel();
    }
    mDependent.clear();
}

void CPUTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleCPUTask(ref<Task>());
}

void HDDTask::scheduleTaskNow() {
    TaskScheduler::sGetInstance()->scheduleHDDTask(ref<Task>());
}

void ContainerTask::scheduleTaskNow() {
    scheduleReadyChildren();
    TaskScheduler::sGetInstance()->scheduleCPUTask(ref<Task>());
}

void ContainerTask::scheduleReadyChildren() {
    for(int i = 0; i < mCPUTasks.count(); i++) {
        const auto iTask = mCPUTasks.at(i);
        if(iTask->readyToBeProcessed()) {
            mProcessingTasks << iTask;
            const auto cTask = SPtrCreate(CustomTask)(
            [iTask]() {
                iTask->aboutToProcess();
            },
            [iTask]() {
                iTask->processTask();
            },
            [iTask, this]() {
                iTask->afterProcessingAsContainerStep();
                scheduleReadyChildren();
            });
            cTask->addDependent(this);
            TaskScheduler::sGetInstance()->scheduleCPUTask(cTask);
            mCPUTasks.removeAt(i);
        }
    }
    for(int i = 0; i < mHDDTasks.count(); i++) {
        const auto& iTask = mHDDTasks.at(i);
        if(iTask->readyToBeProcessed()) {
            mProcessingTasks << iTask;
            const auto cTask = SPtrCreate(CustomTask)(
            [iTask]() {
                iTask->aboutToProcess();
            },
            [iTask]() {
                iTask->processTask();
            },
            [iTask, this]() {
                iTask->afterProcessingAsContainerStep();
                scheduleReadyChildren();
            });
            cTask->addDependent(this);
            TaskScheduler::sGetInstance()->scheduleHDDTask(cTask);
            mHDDTasks.removeAt(i);
        }
    }
}
