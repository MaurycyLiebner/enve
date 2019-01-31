#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>
#include <QEventLoop>
#include "smartPointers/sharedpointerdefs.h"
class TaskExecutor;

class _Task : public StdSelfRef {
public:
    _Task();
    ~_Task() {
        tellDependentThatFinished();
        tellNextDependentThatFinished();
    }

    void setCurrentTaskExecutor(TaskExecutor *taskExecutor);

    virtual void beforeProcessingStarted();

    virtual void _processUpdate() = 0;

    void finishedProcessing();

    bool isBeingProcessed();

    void waitTillProcessed();

    bool readyToBeProcessed();

    virtual void clear();

    void addDependent(_Task * const updatable);

    bool finished();

    void decDependencies();
    void incDependencies();
protected:
    virtual void afterProcessingFinished();

    bool mFinished = false;
    bool mBeingProcessed = false;

    QPointer<TaskExecutor> mCurrentTaskExecutor;
    stdsptr<_Task> mSelfRef;
private:
    void tellDependentThatFinished();
    void tellNextDependentThatFinished();
    int mNDependancies = 0;
    QList<stdptr<_Task>> mNextExecutionDependent;
    QList<stdptr<_Task>> mCurrentExecutionDependent;
};

class _ScheduledTask : public _Task {
public:
    _ScheduledTask() {
        mFinished = true;
    }

    void beforeProcessingStarted();

    virtual void taskQued();

    bool scheduleTask();

    virtual bool shouldUpdate() {
        return true;
    }

    bool isQued() { return mTaskQued; }
    bool isScheduled() { return mTaskScheduled; }

    void clear();

    virtual bool needsGpuProcessing() const { return false; }
protected:
    virtual void scheduleTaskNow();
    bool mTaskScheduled = false;
    bool mTaskQued = false;
};

class _HDDTask : public _ScheduledTask {
    friend class StdSelfRef;
protected:
    void scheduleTaskNow();
};

class CustomCPUTask : public _ScheduledTask {
    friend class StdSelfRef;
public:
    void beforeProcessingStarted() {
        _ScheduledTask::beforeProcessingStarted();
        if(mBefore) mBefore();
    }

    void _processUpdate() {
        if(mRun) mRun();
    }

protected:
    void afterProcessingFinished() {
        if(mAfter) mAfter();
    }

    CustomCPUTask(const std::function<void(void)>& before,
                  const std::function<void(void)>& run,
                  const std::function<void(void)>& after) :
        mBefore(before), mRun(run), mAfter(after) {}
private:
    const std::function<void(void)> mBefore;
    const std::function<void(void)> mRun;
    const std::function<void(void)> mAfter;
};

#endif // UPDATABLE_H
