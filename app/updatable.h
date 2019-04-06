#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>
#include <QEventLoop>
#include "smartPointers/sharedpointerdefs.h"
class TaskExecutor;

class _Task : public StdSelfRef {
public:
    enum State {
        CREATED = 0,
        PROCESSING = 10,
        FINISHED = 20
    };

    _Task();
    ~_Task() {
        tellDependentThatFinished();
        tellNextDependentThatFinished();
    }

    bool isActive() { return mState != CREATED && mState != FINISHED; }

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

    State mState = CREATED;

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
    _ScheduledTask() {}

    virtual void taskQued();

    bool scheduleTask();

    virtual bool canSchedule() {
        return true;
    }

    bool isQued() { return mState == QUED; }
    bool isScheduled() { return mState == SCHEDULED; }

    virtual bool needsGpuProcessing() const { return false; }
protected:
    static State SCHEDULED, QUED;
    virtual void scheduleTaskNow();
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
