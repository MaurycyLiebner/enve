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
        const auto taskExecutor = new CPUExecController(this);
        connect(taskExecutor, &ExecController::finishedTaskSignal,
                this, &TaskScheduler::afterCPUTaskFinished);

        mCPUExecutors << taskExecutor;
        mFreeCPUExecs << taskExecutor;
    }

    mHDDExecutor = new HDDExecController;
    mHDDExecs << mHDDExecutor;
    connect(mHDDExecutor, &ExecController::finishedTaskSignal,
            this, &TaskScheduler::afterHDDTaskFinished);
    connect(mHDDExecutor, &HDDExecController::HDDPartFinished,
            this, &TaskScheduler::switchToBackupHDDExecutor);

    mFreeBackupHDDExecs << createNewBackupHDDExecutor();
}

TaskScheduler::~TaskScheduler() {
    for(const auto& exec : mCPUExecutors) {
        exec->quit();
        exec->wait();
    }
    for(const auto& exec : mHDDExecs) {
        exec->quit();
        exec->wait();
    }
}

void TaskScheduler::initializeGPU() {
    try {
        mGpuPostProcessor.initialize();
    } catch(...) {
        RuntimeThrow("Failed to initialize gpu for post-processing.");
    }

    connect(&mGpuPostProcessor, &GpuPostProcessor::finished,
            this, &TaskScheduler::processNextTasks);
    connect(&mGpuPostProcessor, &GpuPostProcessor::processedAll,
            this, &TaskScheduler::callAllTasksFinishedFunc);
}

void TaskScheduler::scheduleCPUTask(const stdsptr<Task>& task) {
    mScheduledCPUTasks << task;
}

void TaskScheduler::scheduleHDDTask(const stdsptr<Task>& task) {
    mScheduledHDDTasks << task;
}

void TaskScheduler::scheduleGPUTask(const stdsptr<Task> &task) {
    mGpuPostProcessor.addToProcess(task);
}

void TaskScheduler::queCPUTask(const stdsptr<Task>& task) {
    if(!task->isQued()) task->taskQued();
    mQuedCPUTasks.addTask(task);
    if(task->readyToBeProcessed()) {
        if(!task->gpuSupported() ||
           !processNextQuedGPUTask()) {
            processNextQuedCPUTask();
        }
    }
}

bool TaskScheduler::shouldQueMoreCPUTasks() const {
    const int nQues = mQuedCPUTasks.countQues();
    const int maxQues = mCPUExecutors.count();
    const bool overflowed = nQues >= maxQues;
    return !mFreeCPUExecs.isEmpty() && !mCPUQueing && !overflowed;
}

bool TaskScheduler::shouldQueMoreHDDTasks() const {
    return mQuedHDDTasks.count() < 2 && mHDDThreadBusy;
}

HDDExecController* TaskScheduler::createNewBackupHDDExecutor() {
    const auto newExec = new HDDExecController;
    connect(newExec, &ExecController::finishedTaskSignal,
            this, &TaskScheduler::afterHDDTaskFinished);
    mHDDExecs << newExec;
    return newExec;
}

void TaskScheduler::queTasks() {
    queScheduledCPUTasks();
    queScheduledHDDTasks();
}

void TaskScheduler::queScheduledCPUTasks() {
    if(!shouldQueMoreCPUTasks()) return;
    mCPUQueing = true;
    mQuedCPUTasks.beginQue();
    for(const auto& it : Document::sInstance->fVisibleScenes) {
        const auto scene = it.first;
        scene->scheduleWaitingTasks();
        scene->queScheduledTasks();
    }
    while(!mScheduledCPUTasks.isEmpty())
        queCPUTask(mScheduledCPUTasks.takeLast());
    mQuedCPUTasks.endQue();
    mCPUQueing = false;

    if(!mQuedCPUTasks.isEmpty()) processNextTasks();
}

void TaskScheduler::queScheduledHDDTasks() {
    if(mHDDThreadBusy) return;
    for(int i = 0; i < mScheduledHDDTasks.count(); i++) {
        const auto task = mScheduledHDDTasks.takeAt(i);
        if(!task->isQued()) task->taskQued();

        mQuedHDDTasks << task;
        tryProcessingNextQuedHDDTask();
    }
}

void TaskScheduler::switchToBackupHDDExecutor() {
    if(!mHDDThreadBusy) return;
    disconnect(mHDDExecutor, &HDDExecController::HDDPartFinished,
               this, &TaskScheduler::switchToBackupHDDExecutor);

    if(mFreeBackupHDDExecs.isEmpty()) {
        mHDDExecutor = createNewBackupHDDExecutor();
    } else {
        mHDDExecutor = mFreeBackupHDDExecs.takeFirst();
    }
    mHDDThreadBusy = false;

    connect(mHDDExecutor, &HDDExecController::HDDPartFinished,
            this, &TaskScheduler::switchToBackupHDDExecutor);
    processNextQuedHDDTask();
}

void TaskScheduler::tryProcessingNextQuedHDDTask() {
    if(!mHDDThreadBusy) processNextQuedHDDTask();
}

void TaskScheduler::afterHDDTaskFinished(
        const stdsptr<Task>& finishedTask,
        ExecController * const controller) {
    if(controller == mHDDExecutor)
        mHDDThreadBusy = false;
    else {
        const auto hddExec = static_cast<HDDExecController*>(controller);
        mFreeBackupHDDExecs << hddExec;
    }
    finishedTask->finishedProcessing();
    processNextTasks();
    if(!HDDTaskBeingProcessed()) queTasks();
    callAllTasksFinishedFunc();
}

void TaskScheduler::processNextQuedHDDTask() {
    if(!mHDDThreadBusy) {
        for(int i = 0; i < mQuedHDDTasks.count(); i++) {
            const auto task = mQuedHDDTasks.at(i);
            if(task->readyToBeProcessed()) {
                task->aboutToProcess(Hardware::HDD);
                const auto hddTask = dynamic_cast<HDDTask*>(task.get());
                if(hddTask) hddTask->setController(mHDDExecutor);
                mQuedHDDTasks.removeAt(i--);
                mHDDThreadBusy = true;
                mHDDExecutor->processTask(task);
                break;
            }
        }
    }

    emit hddUsageChanged(mHDDThreadBusy);
}

void TaskScheduler::processNextTasks() {
    processNextQuedHDDTask();
    processNextQuedGPUTask();
    processNextQuedCPUTask();
    if(shouldQueMoreCPUTasks() || shouldQueMoreHDDTasks())
        callFreeThreadsForCPUTasksAvailableFunc();
}

bool TaskScheduler::processNextQuedGPUTask() {
    if(!mGpuPostProcessor.hasFinished()) return false;
    const auto task = mQuedCPUTasks.takeQuedForGpuProcessing();
    if(task) {
        task->aboutToProcess(Hardware::GPU);
        if(task->getState() > Task::PROCESSING) {
            processNextTasks();
            return true;
        }
        scheduleGPUTask(task);
    }
    emit gpuUsageChanged(!mGpuPostProcessor.hasFinished());
    return task.get();
}

void TaskScheduler::afterCPUTaskFinished(
        const stdsptr<Task>& task,
        ExecController * const controller) {
    mFreeCPUExecs << static_cast<CPUExecController*>(controller);
    if(task->getState() == Task::CANCELED) {}
    else if(task->nextStep()) scheduleCPUTask(task);
    else task->finishedProcessing();
    processNextTasks();
    if(!CPUTasksBeingProcessed()) queTasks();
    callAllTasksFinishedFunc();
}

void TaskScheduler::processNextQuedCPUTask() {
    while(!mFreeCPUExecs.isEmpty() && !mQuedCPUTasks.isEmpty()) {
        const auto task = mQuedCPUTasks.takeQuedForCpuProcessing();
        if(task) {
            task->aboutToProcess(Hardware::CPU);
            if(task->getState() > Task::PROCESSING) {
                return processNextTasks();
            }
            const auto executor = mFreeCPUExecs.takeLast();
            executor->processTask(task);
        } else break;
    }

    const int cUsed = mCPUExecutors.count() - mFreeCPUExecs.count();
    emit cpuUsageChanged(cUsed);
}
