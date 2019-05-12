#include "taskscheduler.h"
#include "Boxes/boundingboxrenderdata.h"
#include "GPUEffects/gpupostprocessor.h"
#include "canvas.h"
#include "taskexecutor.h"
#include <QThread>

TaskScheduler *TaskScheduler::sInstance;

TaskScheduler::TaskScheduler() {
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
    mBackupHDDExecutor = new HDDExecController;
    connect(mHDDExecutor, &ExecController::finishedTaskSignal,
            this, &TaskScheduler::afterHDDTaskFinished);
    connect(mHDDExecutor, &HDDExecController::HDDPartFinished,
            this, &TaskScheduler::switchToBackupHDDExecutor);
}

TaskScheduler::~TaskScheduler() {
    for(const auto& thread : mCPUExecutors) {
        thread->quit();
        thread->wait();
    }
    mHDDExecutor->quit();
    mHDDExecutor->wait();
}

void TaskScheduler::initializeGPU() {
    try {
        mGpuPostProcessor.initialize();
    } catch(...) {
        RuntimeThrow("Failed to initialize gpu for post-processing.");
    }

    connect(&mGpuPostProcessor, &GpuPostProcessor::finished,
            this, &TaskScheduler::tryProcessingNextQuedCPUTask);
    connect(&mGpuPostProcessor, &GpuPostProcessor::processedAll,
            this, &TaskScheduler::finishedAllQuedTasks);
}

void TaskScheduler::scheduleCPUTask(const stdsptr<Task>& task) {
    mScheduledCPUTasks << task;
}

void TaskScheduler::scheduleHDDTask(const stdsptr<Task>& task) {
    mScheduledHDDTasks << task;
}

void TaskScheduler::queCPUTask(const stdsptr<Task>& task) {
    if(!task->isQued()) task->taskQued();
    mQuedCPUTasks.addTask(task);
    if(task->readyToBeProcessed()) processNextQuedCPUTask();
}

bool TaskScheduler::shouldQueMoreCPUTasks() {
    const int nQues = mQuedCPUTasks.countQues();
    const int maxQues = mCPUExecutors.count();
    const bool overflowed = nQues >= maxQues;
    return !mFreeCPUExecs.isEmpty() && !mCPUQueing && !overflowed;
}

void TaskScheduler::queScheduledCPUTasks() {
    if(!shouldQueMoreCPUTasks()) return;
    mCPUQueing = true;
    mQuedCPUTasks.beginQue();
    if(mCurrentCanvas) {
        mCurrentCanvas->scheduleWaitingTasks();
        mCurrentCanvas->queScheduledTasks();
    }
    while(!mScheduledCPUTasks.isEmpty())
        queCPUTask(mScheduledCPUTasks.takeLast());
    mQuedCPUTasks.endQue();
    mCPUQueing = false;

    if(!mQuedCPUTasks.isEmpty()) processNextQuedCPUTask();
}

void TaskScheduler::queScheduledHDDTasks() {
    if(!mHDDThreadBusy) {
        for(int i = 0; i < mScheduledHDDTasks.count(); i++) {
            const auto &task = mScheduledHDDTasks.takeAt(i);
            if(!task->isQued()) task->taskQued();

            mQuedHDDTasks << task;
            tryProcessingNextQuedHDDTask();
        }
    }
}

void TaskScheduler::switchToBackupHDDExecutor() {
    if(mBackupHDDThreadBusy) return;
    const auto hddExecutor = mHDDExecutor;
    mHDDExecutor = mBackupHDDExecutor;
    mBackupHDDExecutor = hddExecutor;
    const auto hddThreadBusy = mHDDThreadBusy;
    mHDDThreadBusy = mBackupHDDThreadBusy;
    mBackupHDDThreadBusy = hddThreadBusy;

    disconnect(mBackupHDDExecutor, &HDDExecController::HDDPartFinished,
               this, &TaskScheduler::switchToBackupHDDExecutor);
    connect(mHDDExecutor, &HDDExecController::HDDPartFinished,
            this, &TaskScheduler::switchToBackupHDDExecutor);
    processNextQuedHDDTask();
}

void TaskScheduler::tryProcessingNextQuedHDDTask() {
    if(!mHDDThreadBusy) processNextQuedHDDTask();
}

void TaskScheduler::tryProcessingNextQuedCPUTask() {
    if(!mFreeCPUExecs.isEmpty()) processNextQuedCPUTask();
}

void TaskScheduler::afterHDDTaskFinished(
        const stdsptr<Task>& finishedTask,
        ExecController * const controller) {
    if(controller == mHDDExecutor) mHDDThreadBusy = false;
    else mBackupHDDThreadBusy = false;
    if(finishedTask) finishedTask->finishedProcessing();
    if(!mFreeCPUExecs.isEmpty() && !mQuedCPUTasks.isEmpty()) {
        processNextQuedCPUTask();
    }
    processNextQuedHDDTask();
}

void TaskScheduler::processNextQuedHDDTask() {
    if(mHDDThreadBusy) return;
    if(mQuedHDDTasks.isEmpty()) {
        callAllQuedHDDTasksFinishedFunc();
//        if(!mRenderingPreview) {
//            callUpdateSchedulers();
//        }
//        if(!mFreeCPUThreads.isEmpty() && !mQuedCPUTasks.isEmpty()) {
//            processNextQuedCPUTask(mFreeCPUThreads.takeFirst(), nullptr);
//        }
    } else {
        for(int i = 0; i < mQuedHDDTasks.count(); i++) {
            const auto task = mQuedHDDTasks.at(i);
            if(task->readyToBeProcessed()) {
                task->aboutToProcess();
                const auto hddTask = dynamic_cast<HDDTask*>(task.get());
                if(hddTask) hddTask->setController(mHDDExecutor);
                mQuedHDDTasks.removeAt(i--);
                mHDDThreadBusy = true;
                mHDDExecutor->processTask(task);
                return;
            }
        }
    }
}
#include "GUI/usagewidget.h"
#include "GUI/mainwindow.h"
void TaskScheduler::afterCPUTaskFinished(
        const stdsptr<Task>& task,
        ExecController * const controller) {
    mFreeCPUExecs << static_cast<CPUExecController*>(controller);
    if(task) {
        if(task->getState() != Task::CANCELED) {
            if(task->needsGpuProcessing()) {
                const auto gpuProcess =
                        SPtrCreate(BoxRenderDataScheduledPostProcess)(
                            GetAsSPtr(task, BoundingBoxRenderData));
                mGpuPostProcessor.addToProcess(gpuProcess);
            } else {
                task->finishedProcessing();
            }
        }
    }
    processNextQuedCPUTask();
}

void TaskScheduler::processNextQuedCPUTask() {
    if(mQuedCPUTasks.isEmpty() && !mCPUQueing) {
        callAllQuedCPUTasksFinishedFunc();
        if(mGpuPostProcessor.hasFinished())
            emit finishedAllQuedTasks();
    } else {
        while(!mFreeCPUExecs.isEmpty()) {
            const auto task = mQuedCPUTasks.takeQuedForProcessing();
            if(task) {
                task->aboutToProcess();
                const auto executor = mFreeCPUExecs.takeLast();
                executor->processTask(task);
            } else break;
        }

        if(shouldQueMoreCPUTasks())
            callFreeThreadsForCPUTasksAvailableFunc();
    }
#ifdef QT_DEBUG
    auto usageWidget = MainWindow::getInstance()->getUsageWidget();
    const int cUsed = mCPUExecutors.count() - mFreeCPUExecs.count();
    usageWidget->setThreadsUsage(cUsed);
#endif
}
