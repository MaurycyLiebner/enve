#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H
#include <QObject>
#include "Tasks/updatable.h"
#include "Tasks/gpupostprocessor.h"
class Canvas;
class CpuExecController;
class HddExecController;
class ExecController;

class Que {
    friend class QueHandler;
public:
    explicit Que() {}
    Que(const Que&) = delete;
    Que& operator=(const Que&) = delete;

    ~Que() {
        for(const auto& cpuTask : mQued) cpuTask->cancel();
        for(const auto& gcpuTask : mGpuPreffered) gcpuTask->cancel();
        for(const auto& gpuTask : mGpuOnly) gpuTask->cancel();
    }
protected:
    int countQued() const { return mQued.count() +
                                   mGpuPreffered.count() +
                                   mGpuOnly.count(); }
    bool allDone() const { return countQued() == 0; }
    void addTask(const stdsptr<eTask>& task) {
        const auto hwSupport = task->hardwareSupport();
        if(hwSupport == HardwareSupport::gpuOnly) mGpuOnly << task;
        else if(hwSupport == HardwareSupport::gpuPreffered) mGpuPreffered << task;
        else mQued << task;
    }

    stdsptr<eTask> takeQuedForCpuProcessing() {
        for(int i = 0; i < mQued.count(); i++) {
            const auto& task = mQued.at(i);
            if(task->readyToBeProcessed()) return mQued.takeAt(i);
        }
        for(int i = 0; i < mGpuPreffered.count(); i++) {
            const auto& task = mGpuPreffered.at(i);
            if(task->readyToBeProcessed()) return mGpuPreffered.takeAt(i);
        }
        return nullptr;
    }

    stdsptr<eTask> takeQuedForGpuProcessing() {
        for(int i = 0; i < mGpuOnly.count(); i++) {
            const auto& task = mGpuOnly.at(i);
            if(task->readyToBeProcessed()) return mGpuOnly.takeAt(i);
        }
        for(int i = 0; i < mGpuPreffered.count(); i++) {
            const auto& task = mGpuPreffered.at(i);
            if(task->readyToBeProcessed()) return mGpuPreffered.takeAt(i);
        }
        for(int i = 0; i < mQued.count(); i++) {
            const auto& task = mQued.at(i);
            if(!task->readyToBeProcessed()) continue;
            if(task->hardwareSupport() == HardwareSupport::cpuOnly) continue;
            return mQued.takeAt(i);
        }
        return nullptr;
    }
private:
    QList<stdsptr<eTask>> mGpuOnly;
    QList<stdsptr<eTask>> mGpuPreffered;
    QList<stdsptr<eTask>> mQued;
};

class QueHandler {
public:
    int countQues() const { return mQues.count(); }
    bool isEmpty() const { return mQues.isEmpty(); }

    void clear() {
        mQues.clear();
        mCurrentQue = nullptr;
    }

    stdsptr<eTask> takeQuedForGpuProcessing() {
        int queId = 0;
        for(const auto& que : mQues) {
            const auto task = que->takeQuedForGpuProcessing();
            if(task) {
                if(que->allDone()) queDone(que.get(), queId);
                return task;
            }
            queId++;
        }
        return nullptr;
    }

    stdsptr<eTask> takeQuedForCpuProcessing() {
        int queId = 0;
        for(const auto& que : mQues) {
            const auto task = que->takeQuedForCpuProcessing();
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

    void addTask(const stdsptr<eTask>& task) {
        if(!mCurrentQue) RuntimeThrow("Cannot add task when there is no active que.");
        mCurrentQue->addTask(task);
    }

    void addTaskFastTrack(const stdsptr<eTask>& task) {
        if(mCurrentQue) {
            addTask(task);
        } else {
            if(mQues.isEmpty()) beginQue();
            else mCurrentQue = mQues.first().get();
            addTask(task);
            endQue();
        }
    }

    void endQue() {
        if(!mCurrentQue) return;
        if(mCurrentQue->allDone()) mQues.removeLast();
        mCurrentQue = nullptr;
    }
private:
    void queDone(const Que * const que, const int queId) {
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
    static TaskScheduler* sInstance;

    static TaskScheduler * sGetInstance() { return sInstance; }
    static void sSetFreeThreadsForCpuTasksAvailableFunc(
            const std::function<void(void)>& func) {
        sInstance->setFreeThreadsForCpuTasksAvailableFunc(func);
    }

    static void sSetAllTasksFinishedFunc(
            const std::function<void(void)>& func) {
        sInstance->setAllTasksFinishedFunc(func);
    }
    static void sClearAllFinishedFuncs() {
        sSetFreeThreadsForCpuTasksAvailableFunc(nullptr);
        sSetAllTasksFinishedFunc(nullptr);
    }

    static bool sAllTasksFinished() {
        return sInstance->allQuedTasksFinished();
    }
    static bool sAllQuedCpuTasksFinished() {
        return sInstance->allQuedCpuTasksFinished();
    }

    static bool sCpuTasksBeingProcessed() {
        return sInstance->cpuTasksBeingProcessed();
    }

    static bool sHddTasksBeingProcessed() {
        return sInstance->hddTaskBeingProcessed();
    }

    static bool sAllQuedHddTasksFinished() {
        return sInstance->allQuedHddTasksFinished();
    }

    static void sClearTasks() {
        sInstance->clearTasks();
    }

    void initializeGpu();

    void queTasks();
    void queCpuTaskFastTrack(const stdsptr<eTask>& task);

    void scheduleCpuTask(const stdsptr<eTask> &task);
    void scheduleHddTask(const stdsptr<eTask> &task);
    void scheduleGpuTask(const stdsptr<eTask>& task);

    void clearTasks() {
        for(const auto& cpuTask : mScheduledCpuTasks)
            cpuTask->cancel();
        mScheduledCpuTasks.clear();

        for(const auto& hddTask : mScheduledHddTasks)
            hddTask->cancel();
        mScheduledHddTasks.clear();

        mQuedCpuTasks.clear();

        for(const auto& hddTask : mQuedHddTasks)
            hddTask->cancel();
        mQuedHddTasks.clear();

        mGpuPostProcessor.clear();

        callAllTasksFinishedFunc();
    }

    void switchToBackupHddExecutor();

    void afterHddTaskFinished(const stdsptr<eTask>& finishedTask,
                              ExecController * const controller);

    void afterCpuTaskFinished(const stdsptr<eTask>& task,
                              ExecController * const controller);

    void setFreeThreadsForCpuTasksAvailableFunc(
            const std::function<void(void)>& func) {
        mFreeThreadsForCpuTasksAvailableFunc = func;
    }

    void setAllTasksFinishedFunc(
            const std::function<void(void)>& func) {
        mAllTasksFinishedFunc = func;
    }

    bool allQuedTasksFinished() const {
        return allQuedCpuTasksFinished() &&
               allQuedHddTasksFinished() &&
               allQuedGpuTasksFinished();
    }

    bool allQuedGpuTasksFinished() const {
        return mGpuPostProcessor.allDone();
    }

    bool allQuedCpuTasksFinished() const {
        return !cpuTasksBeingProcessed() && mQuedCpuTasks.isEmpty();
    }

    bool allQuedHddTasksFinished() const {
        return !hddTaskBeingProcessed() && mQuedHddTasks.isEmpty();
    }

    bool cpuTasksBeingProcessed() const {
        return busyCpuThreads() > 0;
    }

    bool hddTaskBeingProcessed() const {
        return busyHddThreads() > 0;
    }

    int busyHddThreads() const {
        const int totalThreads = mHddExecs.count();
        const int freeBackup = mFreeBackupHddExecs.count();
        const int freeMain = mHddThreadBusy ? 0 : 1;
        return totalThreads - freeBackup - freeMain;
    }

    int busyCpuThreads() const {
        return mCpuExecutors.count() - mFreeCpuExecs.count();
    }

    void afterCpuGpuTaskFinished();
signals:
    void finishedAllQuedTasks() const;
    void hddUsageChanged(bool);
    void gpuUsageChanged(bool);
    void cpuUsageChanged(int);
private:
    void queCpuTask(const stdsptr<eTask> &task);
    void queScheduledCpuTasks();
    void queScheduledHddTasks();

    void processNextQuedHddTask();
    void processNextQuedCpuTask();
    bool processNextQuedGpuTask();
    void processNextTasks();

    void tryProcessingNextQuedHddTask();

    bool shouldQueMoreCpuTasks() const;
    bool shouldQueMoreHddTasks() const;

    void callFreeThreadsForCpuTasksAvailableFunc() const {
        if(mFreeThreadsForCpuTasksAvailableFunc) {
            mFreeThreadsForCpuTasksAvailableFunc();
        }
    }

    void callAllTasksFinishedFunc() const {
        if(allQuedTasksFinished()) {
            if(mAllTasksFinishedFunc) {
                mAllTasksFinishedFunc();
            }
            emit finishedAllQuedTasks();
        }
    }

    bool mHddThreadBusy = false;

    bool mCpuQueing = false;
    QueHandler mQuedCpuTasks;

    QList<stdsptr<eTask>> mScheduledCpuTasks;
    QList<stdsptr<eTask>> mScheduledHddTasks;
    QList<stdsptr<eTask>> mQuedHddTasks;

    HddExecController *createNewBackupHddExecutor();

    HddExecController *mHddExecutor = nullptr;
    QList<HddExecController*> mFreeBackupHddExecs;
    QList<HddExecController*> mHddExecs;

    QList<CpuExecController*> mFreeCpuExecs;
    QList<CpuExecController*> mCpuExecutors;

    std::function<void(void)> mFreeThreadsForCpuTasksAvailableFunc;
    std::function<void(void)> mAllTasksFinishedFunc;

    GpuPostProcessor mGpuPostProcessor;
};

#endif // TASKSCHEDULER_H
