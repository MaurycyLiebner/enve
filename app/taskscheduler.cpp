#include "taskscheduler.h"
#include "Boxes/boundingboxrenderdata.h"
#include "gpupostprocessor.h"
#include "canvas.h"
#include "taskexecutor.h"
#include <QThread>

TaskScheduler *TaskScheduler::sInstance;

TaskScheduler::TaskScheduler() {
    sInstance = this;
    int numberThreads = qMax(1, QThread::idealThreadCount());
    for(int i = 0; i < numberThreads; i++) {
        QThread *taskExecutorThread = new QThread(this);
        TaskExecutor *taskExecutor = new TaskExecutor(i);
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
    foreach(QThread *thread, mExecutorThreads) {
        thread->quit();
        thread->wait();
        delete thread;
    }
    foreach(TaskExecutor *taskExecutor, mCPUTaskExecutors) {
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
        gPrintExceptionCritical(e, "Failed to initialize gpu for post-processing.\n");
    }

    connect(&mGpuPostProcessor, &GpuPostProcessor::finished,
            this, &TaskScheduler::tryProcessingNextQuedCPUTask);
    connect(&mGpuPostProcessor, &GpuPostProcessor::processedAll,
            this, &TaskScheduler::finishedAllQuedTasks);
}

void TaskScheduler::scheduleCPUTask(const stdsptr<_ScheduledTask>& task) {
    mScheduledCPUTasks << task;
}

void TaskScheduler::queHDDTask(const stdsptr<_ScheduledTask>& task) {
    mQuedHDDTasks << task;
    processNextQuedHDDTask(mCPUTaskExecutors.count(), nullptr);
}

void TaskScheduler::queScheduledCPUTasks() {
    if(mBusyCPUThreads.isEmpty() && mQuedCPUTasks.isEmpty()) {
        if(mCurrentCanvas) {
            mCurrentCanvas->processSchedulers();
            mCurrentCanvas->queScheduledTasks();
        }
        foreach(const auto &task, mScheduledCPUTasks) {
            if(!task->isAwaitingUpdate()) {
                task->schedulerProccessed();
            }

            mQuedCPUTasks << task;
            tryProcessingNextQuedCPUTask();
        }
        mScheduledCPUTasks.clear();
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
    if(finishedTask != nullptr) {
        finishedTask->updateFinished();
    }
    if(mQuedHDDTasks.isEmpty()) {
        callAllQuedHDDTasksFinishedFunc();
//        if(!mRenderingPreview) {
//            callUpdateSchedulers();
//        }
        if(!mFreeCPUThreads.isEmpty() && !mQuedCPUTasks.isEmpty()) {
            processNextQuedCPUTask(mFreeCPUThreads.takeFirst(), nullptr);
        }
    } else {
        for(int i = 0; i < mQuedHDDTasks.count(); i++) {
            auto task = mQuedHDDTasks.at(i).get();
            if(task->readyToBeProcessed()) {
                task->setCurrentTaskExecutor(mHDDExecutor);
                task->beforeUpdate();
                emit processHDDTask(task, mCPUTaskExecutors.count());
                mQuedHDDTasks.takeAt(i);
                i--;
                mHDDThreadBusy = true;
                return;
            }
        }
    }
}

void TaskScheduler::processNextQuedCPUTask(
        const int &finishedThreadId,
        _ScheduledTask *const finishedTask) {
    if(finishedTask != nullptr) {
        mBusyCPUThreads.removeOne(finishedThreadId);
        if(finishedTask->needsGpuProcessing()) {
            auto gpuProcess =
                    SPtrCreate(BoxRenderDataScheduledPostProcess)(
                        GetAsSPtr(finishedTask, BoundingBoxRenderData));
            mGpuPostProcessor.addToProcess(gpuProcess);
        } else {
            finishedTask->updateFinished();
        }
    }
    if(mQuedCPUTasks.isEmpty()) {
        mFreeCPUThreads << finishedThreadId;
        callAllQuedCPUTasksFinishedFunc();
        if(mGpuPostProcessor.hasFinished()) {
            emit finishedAllQuedTasks();
        }
    } else {
        int threadId = finishedThreadId;
        for(int i = 0; i < mQuedCPUTasks.count(); i++) {
            _ScheduledTask *updatablaT =
                    mQuedCPUTasks.at(i).get();
            if(updatablaT->readyToBeProcessed()) {
                updatablaT->setCurrentTaskExecutor(
                            mCPUTaskExecutors.at(threadId));
                updatablaT->beforeUpdate();
                mQuedCPUTasks.removeAt(i);
                emit processCPUTask(updatablaT, threadId);
                mBusyCPUThreads << threadId;
                i--;
                //return;
                if(mFreeCPUThreads.isEmpty() || mQuedCPUTasks.isEmpty()) {
//                    UsageWidget* usageWidget = MainWindow::getInstance()->getUsageWidget();
//                    if(usageWidget != nullptr)
//                        usageWidget->setThreadsUsage(mThreadsUsed);
                    return;
                }
                threadId = mFreeCPUThreads.takeFirst();
            }
        }
        mFreeCPUThreads << threadId;
        callAllQuedCPUTasksFinishedFunc();
    }
//    UsageWidget* usageWidget = MainWindow::getInstance()->getUsageWidget();
//    if(usageWidget == nullptr) return;
//    usageWidget->setThreadsUsage(mThreadsUsed);
}
