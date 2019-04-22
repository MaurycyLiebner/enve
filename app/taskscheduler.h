#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H
#include <QObject>
#include "updatable.h"
#include "GPUEffects/gpupostprocessor.h"
class Canvas;
class ExecController;

class Que {
    friend class QueHandler;
public:
    explicit Que() {}
    Que(const Que&) = delete;
    Que& operator=(const Que&) = delete;

    ~Que() {
        for(const auto& cpuTask : mQued) cpuTask->cancel();
    }
protected:
    int countQued() const { return mQued.count(); }
    bool allDone() const { return countQued() == 0; }
    void addTask(const stdsptr<Task>& task) {
        for(int i = 0; i < mQued.count(); i++) {
            const auto& iTask = mQued.at(i);
            if(iTask == task) Q_ASSERT(false);
        }
        mQued << task;
    }

    stdsptr<Task> takeQuedForProcessing() {
        for(int i = 0; i < mQued.count(); i++) {
            const auto& task = mQued.at(i);
            if(task->readyToBeProcessed()) return mQued.takeAt(i);
        }
        return nullptr;
    }
private:
    QList<stdsptr<Task>> mQued;
};

class QueHandler {
public:
    int countQues() const { return mQues.count(); }
    bool isEmpty() const { return mQues.isEmpty(); }

    void clear() {
        mQues.clear();
        mCurrentQue = nullptr;
    }

    stdsptr<Task> takeQuedForProcessing() {
        int queId = 0;
        for(const auto& que : mQues) {
            const auto task = que->takeQuedForProcessing();
            if(task) {
                if(que->allDone()) queDone(que.get(), queId);
                return task;
            }
            queId++;
        }
        return nullptr;
    }

    void beginQue() {
        if(mCurrentQue) RuntimeThrow("Previous list not ended");
        mQues << std::make_shared<Que>();
        mCurrentQue = mQues.last().get();
    }

    void addTask(const stdsptr<Task>& task) {
        if(!mCurrentQue) RuntimeThrow("Cannot add task when there is no active que.");
        mCurrentQue->addTask(task);
    }

    void endQue() {
        if(!mCurrentQue) return;
        if(mCurrentQue->allDone()) mQues.removeLast();
        mCurrentQue = nullptr;
    }
private:
    void queDone(const Que * const que, const int& queId) {
        if(que == mCurrentQue) return;
        mQues.removeAt(queId);
    }

    QList<stdsptr<Que>> mQues;
    Que * mCurrentQue = nullptr;
};

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
        sSetFreeThreadsForCPUTasksAvailableFunc(nullptr);
        sSetAllQuedCPUTasksFinishedFunc(nullptr);
        sSetAllQuedHDDTasksFinishedFunc(nullptr);
        sSetAllQuedTasksFinishedFunc(nullptr);
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

    void scheduleCPUTask(const stdsptr<Task> &task);
    void queCPUTask(const stdsptr<Task> &task);
    void queScheduledCPUTasks();

    void scheduleHDDTask(const stdsptr<Task> &task);
    void queScheduledHDDTasks();

    void clearTasks() {
        for(const auto& cpuTask : mScheduledCPUTasks)
            cpuTask->cancel();
        mScheduledCPUTasks.clear();

        for(const auto& hddTask : mScheduledHDDTasks)
            hddTask->cancel();
        mScheduledHDDTasks.clear();

        mQuedCPUTasks.clear();

        for(const auto& hddTask : mQuedHDDTasks)
            hddTask->cancel();
        mQuedHDDTasks.clear();

        if(!mHDDThreadBusy) {
            callAllQuedHDDTasksFinishedFunc();
        }
        if(!CPUTasksBeingProcessed()) {
            callAllQuedCPUTasksFinishedFunc();
        }
    }

    void afterHDDTaskFinished(const stdsptr<Task>& finishedTask,
                              ExecController * const controller);
    void processNextQuedHDDTask();

    void afterCPUTaskFinished(const stdsptr<Task>& task,
                              ExecController * const controller);
    void processNextQuedCPUTask();

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
        return !CPUTasksBeingProcessed() && mQuedCPUTasks.isEmpty();
    }

    bool allQuedHDDTasksFinished() const {
        return !mHDDThreadBusy && mQuedHDDTasks.isEmpty();
    }

    bool CPUTasksBeingProcessed() const {
        return busyCPUThreads() > 0;
    }

    bool HDDTaskBeingProcessed() const {
        return !mHDDThreadBusy;
    }

    int busyCPUThreads() const {
        return mCPUTaskExecutors.count() - mFreeCPUExecs.count();
    }
signals:
    void processCPUTask(Task*, int);
    void processHDDTask(Task*, int);
    void finishedAllQuedTasks();
private:
    static TaskScheduler* sInstance;

    void tryProcessingNextQuedCPUTask();
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

    QList<ExecController*> mFreeCPUExecs;

    bool mHDDThreadBusy = false;

    bool mCPUQueing = false;
    QueHandler mQuedCPUTasks;

    QList<stdsptr<Task>> mScheduledCPUTasks;
    QList<stdsptr<Task>> mScheduledHDDTasks;
    QList<stdsptr<Task>> mQuedHDDTasks;

    ExecController *mHDDExecutor = nullptr;
    QList<ExecController*> mCPUTaskExecutors;

    std::function<void(void)> mFreeThreadsForCPUTasksAvailableFunc;
    std::function<void(void)> mAllQuedCPUTasksFinishedFunc;
    std::function<void(void)> mAllQuedHDDTasksFinishedFunc;
    std::function<void(void)> mAllQuedTasksFinishedFunc;

    GpuPostProcessor mGpuPostProcessor;

    Canvas* mCurrentCanvas = nullptr;
};

#endif // TASKSCHEDULER_H
