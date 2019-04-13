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
        const auto taskExecutor = new ExecController(this);
        connect(taskExecutor, &ExecController::finishedTaskSignal,
                this, &TaskScheduler::afterCPUTaskFinished);

        mCPUTaskExecutors << taskExecutor;
        mFreeCPUThreads << taskExecutor;
    }

    mHDDExecutor = new ExecController;
    connect(mHDDExecutor, &ExecController::finishedTaskSignal,
            this, &TaskScheduler::afterHDDTaskFinished);
}

TaskScheduler::~TaskScheduler() {
    for(const auto& thread : mCPUTaskExecutors) {
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

void TaskScheduler::scheduleCPUTask(const stdsptr<_ScheduledTask>& task) {
    mScheduledCPUTasks << task;
}

void TaskScheduler::scheduleHDDTask(const stdsptr<_ScheduledTask>& task) {
    mScheduledHDDTasks << task;
}

void TaskScheduler::queCPUTask(const stdsptr<_ScheduledTask>& task) {
    if(!task->isQued()) task->taskQued();
    mQuedCPUTasks << task;
    if(task->readyToBeProcessed()) processNextQuedCPUTask();
}

void TaskScheduler::queScheduledCPUTasks() {
    if(mQuedCPUTasks.listCount() >= mCPUTaskExecutors.count()) return;
    mCPUQueing = true;
    mQuedCPUTasks.beginList();
    if(mCurrentCanvas) {
        mCurrentCanvas->scheduleWaitingTasks();
        mCurrentCanvas->queScheduledTasks();
    }
    while(!mScheduledCPUTasks.isEmpty())
        queCPUTask(mScheduledCPUTasks.takeLast());
    mQuedCPUTasks.endList();
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

void TaskScheduler::tryProcessingNextQuedHDDTask() {
    if(!mHDDThreadBusy) processNextQuedHDDTask();
}

void TaskScheduler::tryProcessingNextQuedCPUTask() {
    if(!mFreeCPUThreads.isEmpty()) processNextQuedCPUTask();
}

void TaskScheduler::afterHDDTaskFinished(
        _ScheduledTask * const finishedTask,
        ExecController * const controller) {
    Q_UNUSED(controller);
    if(mHDDThreadBusy && !finishedTask) return;
    mHDDThreadBusy = false;
    if(finishedTask) finishedTask->finishedProcessing();
    if(!mFreeCPUThreads.isEmpty() && !mQuedCPUTasks.isEmpty()) {
        processNextQuedCPUTask();
    }
    processNextQuedHDDTask();
}

void TaskScheduler::processNextQuedHDDTask() {
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
            auto task = mQuedHDDTasks.at(i).get();
            if(task->readyToBeProcessed()) {
                task->beforeProcessingStarted();
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
        _ScheduledTask * const finishedTask,
        ExecController * const controller) {
    mFreeCPUThreads << controller;
    if(finishedTask) {
        if(finishedTask->needsGpuProcessing()) {
            auto gpuProcess =
                    SPtrCreate(BoxRenderDataScheduledPostProcess)(
                        GetAsSPtr(finishedTask, BoundingBoxRenderData));
            mGpuPostProcessor.addToProcess(gpuProcess);
        } else {
            finishedTask->finishedProcessing();
        }
    }
    processNextQuedCPUTask();
}

void TaskScheduler::processNextQuedCPUTask() {
    if(mQuedCPUTasks.isEmpty() && !mCPUQueing) {
        callAllQuedCPUTasksFinishedFunc();
        if(mGpuPostProcessor.hasFinished())
            emit finishedAllQuedTasks();
    } else if(!mFreeCPUThreads.isEmpty()) {
        for(int i = 0; i < mQuedCPUTasks.count(); i++) {
            const auto task = mQuedCPUTasks.at(i);
            if(task->readyToBeProcessed()) {
                mQuedCPUTasks.removeAt(i--);
                task->beforeProcessingStarted();
                const auto executor = mFreeCPUThreads.takeLast();
                executor->processTask(task.get());
                if(mFreeCPUThreads.isEmpty()) break;
            }
        }

        if(!mFreeCPUThreads.isEmpty() && !mCPUQueing)
            callFreeThreadsForCPUTasksAvailableFunc();
    }
#ifdef QT_DEBUG
    auto usageWidget = MainWindow::getInstance()->getUsageWidget();
    const int cUsed = mCPUTaskExecutors.count() - mFreeCPUThreads.count();
    usageWidget->setThreadsUsage(cUsed);
#endif
}
