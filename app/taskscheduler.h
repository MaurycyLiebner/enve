#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H
#include <QObject>
#include "updatable.h"
#include "GPUEffects/gpupostprocessor.h"
class Canvas;

class TaskScheduler : public QObject {
    Q_OBJECT
public:
    TaskScheduler();
    ~TaskScheduler();
    static TaskScheduler * sGetInstance() { return sInstance; }
    static void sSetFreeThreadsForCPUTasksAvailableFunc(
            const std::function<void(void)>& func) {
        sInstance->setFreeThreadsForCPUTasksAvailableFunc(func);
    }
    static void sSetAllQuedCPUTasksFinishedFunc(
            const std::function<void(void)>& func) {
        sInstance->setAllQuedCPUTasksFinishedFunc(func);
    }
    static void sSetAllQuedHDDTasksFinishedFunc(
            const std::function<void(void)>& func) {
        sInstance->setAllQuedHDDTasksFinishedFunc(func);
    }
    static void sSetAllQuedTasksFinishedFunc(
            const std::function<void(void)>& func) {
        sInstance->setAllQuedTasksFinishedFunc(func);
    }
    static void sClearAllFinishedFuncs() {
        sInstance->setAllQuedTasksFinishedFunc(nullptr);
        sInstance->setAllQuedCPUTasksFinishedFunc(nullptr);
        sInstance->setAllQuedHDDTasksFinishedFunc(nullptr);
    }
    static bool sAllQuedTasksFinished() {
        return sInstance->allQuedTasksFinished();
    }
    static bool sAllQuedCPUTasksFinished() {
        return sInstance->allQuedCPUTasksFinished();
    }

    static bool sCPUTasksBeingProcessed() {
        return sInstance->CPUTasksBeingProcessed();
    }

    static bool sHDDTasksBeingProcessed() {
        return sInstance->HDDTaskBeingProcessed();
    }

    static bool sAllQuedHDDTasksFinished() {
        return sInstance->allQuedHDDTasksFinished();
    }

    static void sSetCurrentCanvas(Canvas* const canvas) {
        sInstance->setCurrentCanvas(canvas);
    }

    static void sClearTasks() {
        sInstance->clearTasks();
    }

    void initializeGPU();

    void queCPUTask(const stdsptr<_ScheduledTask> &task);
    void scheduleCPUTask(const stdsptr<_ScheduledTask> &task);
    void scheduleHDDTask(const stdsptr<_ScheduledTask> &task);

    void queScheduledCPUTasks();
    void queScheduledHDDTasks();

    void clearTasks() {
        for(const auto& cpuTask : mScheduledCPUTasks) {
            cpuTask->setState(_Task::CANCELED);
        }
        mScheduledCPUTasks.clear();

        for(const auto& hddTask : mScheduledHDDTasks) {
            hddTask->setState(_Task::CANCELED);
        }
        mScheduledHDDTasks.clear();

        for(const auto& cpuTask : mQuedCPUTasks) {
            cpuTask->setState(_Task::CANCELED);
        }
        mQuedCPUTasks.clear();

        for(const auto& hddTask : mQuedHDDTasks) {
            hddTask->setState(_Task::CANCELED);
        }
        mQuedHDDTasks.clear();

        if(!mHDDThreadBusy) {
            callAllQuedHDDTasksFinishedFunc();
        }
        if(mBusyCPUThreads.isEmpty()) {
            callAllQuedCPUTasksFinishedFunc();
        }
    }

    void processNextQuedHDDTask(
            const int &finishedThreadId,
            _ScheduledTask * const finishedTask);
    void processNextQuedCPUTask(
            const int &finishedThreadId,
            _ScheduledTask * const finishedTask);

    void setFreeThreadsForCPUTasksAvailableFunc(
            const std::function<void(void)>& func) {
        mFreeThreadsForCPUTasksAvailableFunc = func;
    }

    void setAllQuedCPUTasksFinishedFunc(
            const std::function<void(void)>& func) {
        mAllQuedCPUTasksFinishedFunc = func;
    }

    void setAllQuedHDDTasksFinishedFunc(
            const std::function<void(void)>& func) {
        mAllQuedHDDTasksFinishedFunc = func;
    }

    void setAllQuedTasksFinishedFunc(
            const std::function<void(void)>& func) {
        mAllQuedTasksFinishedFunc = func;
    }

    void setCurrentCanvas(Canvas * const canvas) {
        mCurrentCanvas = canvas;
    }

    bool allQuedTasksFinished() const {
        return allQuedCPUTasksFinished() && allQuedHDDTasksFinished();
    }

    bool allQuedCPUTasksFinished() const {
        return mBusyCPUThreads.isEmpty() && mQuedCPUTasks.isEmpty();
    }

    bool allQuedHDDTasksFinished() const {
        return !mHDDThreadBusy && mQuedHDDTasks.isEmpty();
    }

    bool CPUTasksBeingProcessed() const {
        return !mBusyCPUThreads.isEmpty();
    }

    bool HDDTaskBeingProcessed() const {
        return !mHDDThreadBusy;
    }
signals:
    void processCPUTask(_ScheduledTask*, int);
    void processHDDTask(_ScheduledTask*, int);
    void finishedAllQuedTasks();
private slots:
    void tryProcessingNextQuedCPUTask();
private:
    static TaskScheduler* sInstance;

    void tryProcessingNextQuedHDDTask();

    void callFreeThreadsForCPUTasksAvailableFunc() const {
        if(mFreeThreadsForCPUTasksAvailableFunc) {
            mFreeThreadsForCPUTasksAvailableFunc();
        }
    }

    void callAllQuedCPUTasksFinishedFunc() const {
        if(mAllQuedCPUTasksFinishedFunc) {
            mAllQuedCPUTasksFinishedFunc();
        }
        if(allQuedTasksFinished()) {
            if(mAllQuedTasksFinishedFunc) {
                mAllQuedTasksFinishedFunc();
            }
        }
    }

    void callAllQuedHDDTasksFinishedFunc() const {
        if(mAllQuedHDDTasksFinishedFunc) {
            mAllQuedHDDTasksFinishedFunc();
        }
        if(allQuedTasksFinished()) {
            if(mAllQuedTasksFinishedFunc) {
                mAllQuedTasksFinishedFunc();
            }
        }
    }

    QList<int> mFreeCPUThreads;
    QList<int> mBusyCPUThreads;

    bool mHDDThreadBusy = false;

    QList<stdsptr<_ScheduledTask>> mScheduledCPUTasks;
    QList<stdsptr<_ScheduledTask>> mQuedCPUTasks;
    QList<stdsptr<_ScheduledTask>> mScheduledHDDTasks;
    QList<stdsptr<_ScheduledTask>> mQuedHDDTasks;

    QList<QThread*> mExecutorThreads;
    QThread *mHDDExecutorThread;

    TaskExecutor *mHDDExecutor = nullptr;
    QList<TaskExecutor*> mCPUTaskExecutors;

    std::function<void(void)> mFreeThreadsForCPUTasksAvailableFunc;
    std::function<void(void)> mAllQuedCPUTasksFinishedFunc;
    std::function<void(void)> mAllQuedHDDTasksFinishedFunc;
    std::function<void(void)> mAllQuedTasksFinishedFunc;

    GpuPostProcessor mGpuPostProcessor;

    Canvas* mCurrentCanvas = nullptr;
};

#endif // TASKSCHEDULER_H
