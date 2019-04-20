#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>
#include <QEventLoop>
#include "smartPointers/sharedpointerdefs.h"
class TaskExecutor;

class _Task : public StdSelfRef {
protected:
    _Task();

    virtual void afterProcessingFinished() {}
    virtual void afterCanceled() {}
public:
    enum State : char {
        CANCELED = 0,
        CREATED = 1,
        PROCESSING = 4,
        FINISHED = 5
    };

    ~_Task() {
        tellDependentThatFinished();
        tellNextDependentThatFinished();
    }

    virtual void beforeProcessingStarted();
    virtual void _processUpdate() = 0;
    virtual void clear();

    bool isActive() { return mState != CREATED && mState != FINISHED; }

    void finishedProcessing();
    bool readyToBeProcessed();

    void addDependent(_Task * const updatable);

    bool finished();

    void decDependencies();
    void incDependencies();

    void setState(const State& state) {
        mState = state;
        if(state == CANCELED) afterCanceled();
    }

    State getState() const {
        return mState;
    }

    void setException(const std::exception_ptr& exception) {
        mUpdateException = exception;
    }

    bool unhandledException() const {
        return static_cast<bool>(mUpdateException);
    }

    std::exception_ptr handleException() {
        std::exception_ptr exc;
        mUpdateException.swap(exc);
        return exc;
    }
protected:
    State mState = CREATED;
private:
    void tellDependentThatFinished();
    void tellNextDependentThatFinished();

    int mNDependancies = 0;
    std::exception_ptr mUpdateException;
    QList<stdptr<_Task>> mNextExecutionDependent;
    QList<stdptr<_Task>> mCurrentExecutionDependent;
};
class Que;
class _ScheduledTask : public _Task {
    friend class TaskScheduler;
    friend class Que;
public:
    _ScheduledTask() {}

    virtual void taskQued();

    bool scheduleTask();

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
