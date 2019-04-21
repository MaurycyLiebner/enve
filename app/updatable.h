#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>
#include <QEventLoop>
#include "smartPointers/sharedpointerdefs.h"
class TaskExecutor;

class Que;

class Task : public StdSelfRef {
    friend class TaskScheduler;
    friend class Que;
protected:
    Task() {}

    virtual void beforeProcessing() {}
    virtual void afterProcessing() {}
    virtual void afterCanceled() {}
    virtual void scheduleTaskNow();
public:
    enum State : char {
        CANCELED,
        CREATED,
        SCHEDULED,
        QUED,
        PROCESSING,
        FINISHED
    };

    virtual void _processUpdate() = 0;
    virtual bool needsGpuProcessing() const { return false; }
    virtual void clear();
    virtual void taskQued() { mState = QUED; }

    bool scheduleTask();
    bool isQued() { return mState == QUED; }
    bool isScheduled() { return mState == SCHEDULED; }

    ~Task() {
        tellDependentThatFinished();
        tellNextDependentThatFinished();
    }

    bool isActive() { return mState != CREATED && mState != FINISHED; }

    void aboutToProcess();
    void finishedProcessing();
    bool readyToBeProcessed();

    void addDependent(Task * const updatable);

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
    QList<stdptr<Task>> mNextExecutionDependent;
    QList<stdptr<Task>> mCurrentExecutionDependent;
};

class _HDDTask : public Task {
    friend class StdSelfRef;
protected:
    void scheduleTaskNow();
};

class CustomCPUTask : public Task {
    friend class StdSelfRef;
public:
    void beforeProcessing() {
        Task::beforeProcessing();
        if(mBefore) mBefore();
    }

    void _processUpdate() {
        if(mRun) mRun();
    }

protected:
    void afterProcessing() {
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
