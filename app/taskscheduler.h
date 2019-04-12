#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H
#include <QObject>
#include "updatable.h"
#include "GPUEffects/gpupostprocessor.h"
class Canvas;

template <typename T>
class MultipleList {
public:
    class Iterator {
    public:
        Iterator(const int& id, MultipleList& target) :
            mId(id), mTarget(target) {}
        inline T &operator*() const { return mTarget.at(mId); }
        inline Iterator &operator++() { mId++; return *this; }
        inline bool operator==(const Iterator& other)
        { return mId == other.getId(); }
        inline bool operator!=(const Iterator& other)
        { return !operator==(other); }
    protected:
        int getId() const { return mId; }
    private:
        int mId;
        MultipleList& mTarget;
    };

    int listCount() const {
        return mLists.count();
    }

    bool isEmpty() const {
        return mLists.isEmpty();
    }

    int count() const {
        int count = 0;
        for(const auto& list : mLists)
            count += list.count();
        return count;
    }

    void clear() {
        mLists.clear();
    }

    inline T &at(const int& id) {
        int rId = id;
        for(auto& list : mLists) {
            const int lCount = list.count();
            if(rId < lCount) return list.operator[](rId);
            rId -= lCount;
        }
        RuntimeThrow("Index outside range");
    }

    inline void removeAt(const int& id) {
        int rId = id;
        for(int lId = 0; lId < mLists.count(); lId++) {
            auto& list = mLists[lId];
            const int lCount = list.count();
            if(rId < lCount) {
                list.removeAt(rId);
                if(list.isEmpty()) removeEmptiedList(list, lId);
                return;
            }
            rId -= lCount;
        }
        RuntimeThrow("Index outside range");
    }

    inline T takeAt(const int& id) {
        int rId = id;
        for(int lId = 0; lId < mLists.count(); lId++) {
            auto& list = mLists[lId];
            const int lCount = list.count();
            if(rId < lCount) {
                if(list.count() == 1) {
                    T result = list.at(rId);
                    removeEmptiedList(list, lId);
                    return result;
                }
                return list.takeAt(rId);
            }
            rId -= lCount;
        }
        RuntimeThrow("Index outside range");
    }

    inline void append(const T& t) {
        mCurrentList->append(t);
    }

    inline MultipleList &operator<< (const T &t)
    { append(t); return *this; }

    void beginList() {
        if(mCurrentList) endList();
        mLists << QList<T>();
        mCurrentList = &mLists.last();
    }

    void endList() {
        if(!mCurrentList) return;
        if(mCurrentList->isEmpty()) mLists.removeLast();
        mCurrentList = nullptr;
    }

    Iterator begin() { return Iterator(0, *this); }
    Iterator end() { return Iterator(count(), *this); }
private:
    void removeEmptiedList(const QList<T>& list, const int& listId) {
        if(&list == mCurrentList) return;;
        mLists.removeAt(listId);
    }

    QList<QList<T>> mLists;
    QList<T> * mCurrentList = nullptr;
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
    void afterCPUTaskFinished(
            const int &finishedThreadId,
            _ScheduledTask * const finishedTask);
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

    void queCPUTask(const stdsptr<_ScheduledTask> &task);
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

    MultipleList<stdsptr<_ScheduledTask>> mQuedCPUTasks;
    MultipleList<stdsptr<_ScheduledTask>> mQuedHDDTasks2;

    QList<stdsptr<_ScheduledTask>> mScheduledCPUTasks;
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
