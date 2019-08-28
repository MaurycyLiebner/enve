// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Tasks/taskscheduler.h"
#include "Boxes/boxrenderdata.h"
#include "Tasks/gpupostprocessor.h"
#include "canvas.h"
#include "taskexecutor.h"
#include <QThread>

TaskScheduler *TaskScheduler::sInstance = nullptr;

TaskScheduler::TaskScheduler() {
    Q_ASSERT(!sInstance);
    sInstance = this;
    const int numberThreads = qMax(1, QThread::idealThreadCount());
    for(int i = 0; i < numberThreads; i++) {
        const auto taskExecutor = new CpuExecController(this);
        connect(taskExecutor, &ExecController::finishedTaskSignal,
                this, &TaskScheduler::afterCpuTaskFinished);

        mCpuExecutors << taskExecutor;
        mFreeCpuExecs << taskExecutor;
    }

    mHddExecutor = new HddExecController;
    mHddExecs << mHddExecutor;
    connect(mHddExecutor, &ExecController::finishedTaskSignal,
            this, &TaskScheduler::afterHddTaskFinished);
    connect(mHddExecutor, &HddExecController::hddPartFinished,
            this, &TaskScheduler::switchToBackupHddExecutor);

    mFreeBackupHddExecs << createNewBackupHddExecutor();
}

TaskScheduler::~TaskScheduler() {
    for(const auto& exec : mCpuExecutors) {
        exec->quit();
        exec->wait();
    }
    for(const auto& exec : mHddExecs) {
        exec->quit();
        exec->wait();
    }
}

void TaskScheduler::initializeGpu() {
    try {
        mGpuPostProcessor.initialize();
    } catch(...) {
        RuntimeThrow("Failed to initialize gpu for post-processing.");
    }
}

void TaskScheduler::scheduleCpuTask(const stdsptr<eTask>& task) {
    mScheduledCpuTasks << task;
}

void TaskScheduler::scheduleHddTask(const stdsptr<eTask>& task) {
    mScheduledHddTasks << task;
}

void TaskScheduler::scheduleGpuTask(const stdsptr<eTask> &task) {
    mGpuPostProcessor.addToProcess(task);
}

void TaskScheduler::queCpuTask(const stdsptr<eTask>& task) {
    task->taskQued();
    mQuedCpuTasks.addTask(task);
    if(task->readyToBeProcessed()) {
        if(task->hardwareSupport() == HardwareSupport::cpuOnly ||
           !processNextQuedGpuTask()) {
            processNextQuedCpuTask();
        }
    }
}

void TaskScheduler::queCpuTaskFastTrack(const stdsptr<eTask>& task) {
    task->taskQued();
    mQuedCpuTasks.addTaskFastTrack(task);
    if(task->readyToBeProcessed()) {
        if(task->hardwareSupport() == HardwareSupport::cpuOnly ||
           !processNextQuedGpuTask()) {
            processNextQuedCpuTask();
        }
    }
}

bool TaskScheduler::overflowed() const {
    const int nQues = mQuedCpuTasks.countQues();
    const int maxQues = mCpuExecutors.count();
    return nQues >= maxQues;
}

bool TaskScheduler::shouldQueMoreCpuTasks() const {
    return !mFreeCpuExecs.isEmpty() && !mCpuQueing && !overflowed();
}

bool TaskScheduler::shouldQueMoreHddTasks() const {
    return mQuedHddTasks.count() < 2 && !mCpuQueing &&
            mHddThreadBusy && !overflowed();
}

HddExecController* TaskScheduler::createNewBackupHddExecutor() {
    const auto newExec = new HddExecController;
    connect(newExec, &ExecController::finishedTaskSignal,
            this, &TaskScheduler::afterHddTaskFinished);
    mHddExecs << newExec;
    return newExec;
}

void TaskScheduler::queTasks() {
    queScheduledCpuTasks();
    queScheduledHddTasks();
}

void TaskScheduler::queScheduledCpuTasks() {
    if(!mAlwaysQue && !shouldQueMoreCpuTasks()) return;
    mCpuQueing = true;
    mQuedCpuTasks.beginQue();
    for(const auto& it : Document::sInstance->fVisibleScenes) {
        const auto scene = it.first;
        scene->queTasks();
    }
    while(!mScheduledCpuTasks.isEmpty())
        queCpuTask(mScheduledCpuTasks.takeLast());
    mQuedCpuTasks.endQue();
    mCpuQueing = false;

    if(!mQuedCpuTasks.isEmpty()) processNextTasks();
}

void TaskScheduler::queScheduledHddTasks() {
    if(mHddThreadBusy) return;
    for(int i = 0; i < mScheduledHddTasks.count(); i++) {
        const auto task = mScheduledHddTasks.takeAt(i);
        if(!task->isQued()) task->taskQued();

        mQuedHddTasks << task;
        tryProcessingNextQuedHddTask();
    }
}

void TaskScheduler::switchToBackupHddExecutor() {
    if(!mHddThreadBusy) return;
    disconnect(mHddExecutor, &HddExecController::hddPartFinished,
               this, &TaskScheduler::switchToBackupHddExecutor);

    if(mFreeBackupHddExecs.isEmpty()) {
        mHddExecutor = createNewBackupHddExecutor();
    } else {
        mHddExecutor = mFreeBackupHddExecs.takeFirst();
    }
    mHddThreadBusy = false;

    connect(mHddExecutor, &HddExecController::hddPartFinished,
            this, &TaskScheduler::switchToBackupHddExecutor);
    processNextQuedHddTask();
}

void TaskScheduler::tryProcessingNextQuedHddTask() {
    if(!mHddThreadBusy) processNextQuedHddTask();
}

void TaskScheduler::afterHddTaskFinished(const stdsptr<eTask>& finishedTask,
                                         ExecController * const controller) {
    if(controller == mHddExecutor)
        mHddThreadBusy = false;
    else {
        const auto hddExec = static_cast<HddExecController*>(controller);
        mFreeBackupHddExecs << hddExec;
    }
    finishedTask->finishedProcessing();
    processNextTasks();
    if(!hddTaskBeingProcessed()) queTasks();
    callAllTasksFinishedFunc();
}

void TaskScheduler::processNextQuedHddTask() {
    if(!mHddThreadBusy) {
        for(int i = 0; i < mQuedHddTasks.count(); i++) {
            const auto task = mQuedHddTasks.at(i);
            if(task->readyToBeProcessed()) {
                task->aboutToProcess(Hardware::hdd);
                const auto hddTask = dynamic_cast<eHddTask*>(task.get());
                if(hddTask) hddTask->setController(mHddExecutor);
                mQuedHddTasks.removeAt(i--);
                mHddThreadBusy = true;
                mHddExecutor->processTask(task);
                break;
            }
        }
    }

    emit hddUsageChanged(mHddThreadBusy);
}

void TaskScheduler::processNextTasks() {
    processNextQuedHddTask();
    processNextQuedGpuTask();
    processNextQuedCpuTask();
    if(shouldQueMoreCpuTasks() || shouldQueMoreHddTasks())
        callFreeThreadsForCpuTasksAvailableFunc();
}

bool TaskScheduler::processNextQuedGpuTask() {
    if(!mGpuPostProcessor.allDone()) return false;
    const auto task = mQuedCpuTasks.takeQuedForGpuProcessing();
    if(task) {
        task->aboutToProcess(Hardware::gpu);
        if(task->getState() > eTaskState::processing) {
            processNextTasks();
            return true;
        }
        const int additional = mQuedCpuTasks.taskCount()/(mCpuExecutors.count()*10);
        scheduleGpuTask(task);
        for(int i = 0; i < additional; i++) {
            const auto iTask = mQuedCpuTasks.takeQuedForGpuProcessing();
            if(!iTask) break;
            scheduleGpuTask(iTask);
        }
    }
    emit gpuUsageChanged(!mGpuPostProcessor.allDone());
    return task.get();
}

void TaskScheduler::afterCpuTaskFinished(const stdsptr<eTask>& task,
                                         ExecController * const controller) {
    mFreeCpuExecs << static_cast<CpuExecController*>(controller);
    const bool nextStep = !task->waitingToCancel() && task->nextStep();
    if(nextStep) queCpuTaskFastTrack(task);
    else task->finishedProcessing();
    afterCpuGpuTaskFinished();
}

void TaskScheduler::afterCpuGpuTaskFinished() {
    processNextTasks();
    if(!cpuTasksBeingProcessed()) queTasks();
    callAllTasksFinishedFunc();
}

void TaskScheduler::processNextQuedCpuTask() {
    const int additional = mQuedCpuTasks.taskCount()/(mCpuExecutors.count()*4);
    while(!mFreeCpuExecs.isEmpty() && !mQuedCpuTasks.isEmpty()) {
        const auto task = mQuedCpuTasks.takeQuedForCpuProcessing();
        if(task) {
            task->aboutToProcess(Hardware::cpu);
            if(task->getState() > eTaskState::processing) {
                return processNextTasks();
            }
            const auto executor = mFreeCpuExecs.takeLast();
            executor->processTask(task);
            for(int i = 0; i < additional; i++) {
                const auto iTask = mQuedCpuTasks.takeQuedForCpuProcessing();
                if(!iTask) break;
                executor->processTask(iTask);
            }
        } else break;
    }

    const int cUsed = mCpuExecutors.count() - mFreeCpuExecs.count();
    emit cpuUsageChanged(cUsed);
}
