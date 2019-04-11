#include "taskscheduler.h"
#include "Boxes/boundingboxrenderdata.h"
#include "GPUEffects/gpupostprocessor.h"
#include "canvas.h"
#include "taskexecutor.h"
#include <QThread>

TaskScheduler *TaskScheduler::sInstance;

TaskScheduler::TaskScheduler() {
    sInstance = this;
    int numberThreads = qMax(1, QThread::idealThreadCount());
    for(int i = 0; i < numberThreads; i++) {
        const auto taskExecutorThread = new QThread(this);
        const auto taskExecutor = new TaskExecutor(i);
        taskExecutor->moveToThread(taskExecutorThread);
        connect(taskExecutor, &TaskExecutor::finishedUpdating,
                this, &TaskScheduler::processNextQuedCPUTask);
        connect(this, &TaskScheduler::processCPUTask,
                taskExecutor, &TaskExecutor::updateUpdatable);

        taskExecutorThread->start();

        mCPUTaskExecutors << taskExecutor;
        mExecutorThreads << taskExecutorThread;

        mFreeCPUThreads << i;
    }

    mHDDExecutorThread = new QThread(this);
    mHDDExecutor = new TaskExecutor(numberThreads);
    mHDDExecutor->moveToThread(mHDDExecutorThread);
    connect(mHDDExecutor, &TaskExecutor::finishedUpdating,
            this, &TaskScheduler::processNextQuedHDDTask);
    connect(this, &TaskScheduler::processHDDTask,
            mHDDExecutor, &TaskExecutor::updateUpdatable);

    mHDDExecutorThread->start();
    mExecutorThreads << mHDDExecutorThread;
}

TaskScheduler::~TaskScheduler() {
    for(const auto& thread : mExecutorThreads) {
        thread->quit();
        thread->wait();
        delete thread;
    }
    for(const auto& taskExecutor : mCPUTaskExecutors) {
        delete taskExecutor;
    }
//    mFileControlerThread->quit();
//    mFileControlerThread->wait();
    delete mHDDExecutor;
}

void TaskScheduler::initializeGPU() {
    try {
        mGpuPostProcessor.initialize();
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e, "Failed to initialize gpu for post-processing.");
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
    if(task->finished()) return;
    mQuedCPUTasks2 << task;
    tryProcessingNextQuedCPUTask();
}

void TaskScheduler::queScheduledCPUTasks() {
    if(mQuedCPUTasks2.listCount() >= mCPUTaskExecutors.count()) return;
    mQuedCPUTasks2.beginList();
    if(mCurrentCanvas) {
        mCurrentCanvas->scheduleWaitingTasks();
        mCurrentCanvas->queScheduledTasks();
    }
    for(const auto &task : mScheduledCPUTasks) queCPUTask(task);
    mQuedCPUTasks2.endList();

    mScheduledCPUTasks.clear();
}

void TaskScheduler::queScheduledHDDTasks() {
    if(!mHDDThreadBusy) {
        for(int i = 0; i < mScheduledHDDTasks.count(); i++) {
            const auto &task = mScheduledHDDTasks.at(i);
            if(!task->isQued()) task->taskQued();

            mQuedHDDTasks << task;
            tryProcessingNextQuedHDDTask();
        }
        mScheduledHDDTasks.clear();
    }
}

void TaskScheduler::tryProcessingNextQuedHDDTask() {
    if(!mHDDThreadBusy) {
        processNextQuedHDDTask(mCPUTaskExecutors.count(), nullptr);
    }
}

void TaskScheduler::tryProcessingNextQuedCPUTask() {
    if(!mFreeCPUThreads.isEmpty()) {
        processNextQuedCPUTask(mFreeCPUThreads.takeFirst(), nullptr);
    }
}

void TaskScheduler::processNextQuedHDDTask(
        const int &finishedThreadId,
        _ScheduledTask * const finishedTask) {
    Q_UNUSED(finishedThreadId);
    if(mHDDThreadBusy && !finishedTask) return;
    mHDDThreadBusy = false;
    if(finishedTask) finishedTask->finishedProcessing();
    if(!mFreeCPUThreads.isEmpty() && !mQuedCPUTasks2.isEmpty()) {
        processNextQuedCPUTask(mFreeCPUThreads.takeFirst(), nullptr);
    }
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
                task->setCurrentTaskExecutor(mHDDExecutor);
                task->beforeProcessingStarted();
                emit processHDDTask(task, mCPUTaskExecutors.count());
                mQuedHDDTasks.takeAt(i);
                i--;
                mHDDThreadBusy = true;
                return;
            }
        }
    }
}
#include "GUI/usagewidget.h"
#include "GUI/mainwindow.h"
void TaskScheduler::processNextQuedCPUTask(
        const int &finishedThreadId,
        _ScheduledTask *const finishedTask) {
    mFreeCPUThreads << finishedThreadId;
    if(finishedTask) {
        mBusyCPUThreads.removeOne(finishedThreadId);
        if(finishedTask->needsGpuProcessing()) {
            auto gpuProcess =
                    SPtrCreate(BoxRenderDataScheduledPostProcess)(
                        GetAsSPtr(finishedTask, BoundingBoxRenderData));
            mGpuPostProcessor.addToProcess(gpuProcess);
        } else {
            finishedTask->finishedProcessing();
        }
    }
    if(mQuedCPUTasks2.isEmpty()) {
        callAllQuedCPUTasksFinishedFunc();
        if(mGpuPostProcessor.hasFinished())
            emit finishedAllQuedTasks();
    } else if(!mFreeCPUThreads.isEmpty()) {
        for(int i = 0; i < mQuedCPUTasks2.count(); i++) {
            const auto task = mQuedCPUTasks2.at(i);
            if(task->readyToBeProcessed()) {
                mQuedCPUTasks2.removeAt(i--);
                if(task->finished()) continue;
                task->beforeProcessingStarted();
                if(task->finished()) continue;
                const int threadId = mFreeCPUThreads.takeLast();
                task->setCurrentTaskExecutor(
                            mCPUTaskExecutors.at(threadId));
                emit processCPUTask(task.get(), threadId);
                mBusyCPUThreads << threadId;
                if(mFreeCPUThreads.isEmpty()) break;
            }
        }

        if(!mFreeCPUThreads.isEmpty()) callFreeThreadsForCPUTasksAvailableFunc();
    }
#ifdef QT_DEBUG
    auto usageWidget = MainWindow::getInstance()->getUsageWidget();
    usageWidget->setThreadsUsage(mBusyCPUThreads.count());
#endif
}
