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
    static bool sAllQuedHDDTasksFinished() {
        return sInstance->allQuedHDDTasksFinished();
    }
    static void sSetCurrentCanvas(Canvas* const canvas) {
        sInstance->setCurrentCanvas(canvas);
    }

    void initializeGPU();

    void scheduleCPUTask(const stdsptr<_ScheduledTask> &task);
    void queHDDTask(const stdsptr<_ScheduledTask> &task);

    void clearTasks() {
        mScheduledCPUTasks.clear();
        mQuedHDDTasks.clear();
    }

    void queScheduledCPUTasks();
    void processQuedCPUTasks();

    void processNextQuedHDDTask(
            const int &finishedThreadId,
            _ScheduledTask * const finishedTask);
    void processNextQuedCPUTask(
            const int &finishedThreadId,
            _ScheduledTask * const finishedTask);

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
        return mBusyCPUThreads.isEmpty();
    }

    bool allQuedHDDTasksFinished() const {
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
    QList<stdsptr<_ScheduledTask>> mQuedHDDTasks;

    QList<QThread*> mExecutorThreads;
    QThread *mHDDExecutorThread;

    TaskExecutor *mHDDExecutor = nullptr;
    QList<TaskExecutor*> mCPUTaskExecutors;

    std::function<void(void)> mAllQuedCPUTasksFinishedFunc;
    std::function<void(void)> mAllQuedHDDTasksFinishedFunc;
    std::function<void(void)> mAllQuedTasksFinishedFunc;

    GpuPostProcessor mGpuPostProcessor;

    Canvas* mCurrentCanvas = nullptr;
};

#endif // TASKSCHEDULER_H
