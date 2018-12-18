#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>
#include <QEventLoop>
#include "smartPointers/sharedpointerdefs.h"
class TaskExecutor;

class _Task : public StdSelfRef {
public:
    _Task();

    void setCurrentTaskExecutor(TaskExecutor *taskExecutor);

    virtual void beforeUpdate();

    virtual void _processUpdate() = 0;

    void updateFinished();

    virtual void afterUpdate();

    bool isBeingProcessed();

    void waitTillProcessed();

    bool readyToBeProcessed();

    virtual void clear();

    virtual void addDependent(_Task* updatable);

    bool finished();

    void decDependencies();
    void incDependencies();
protected:
    void tellDependentThatFinished();

    bool mFinished = false;
    bool mBeingProcessed = false;
    int nDependancies = 0;

    QPointer<TaskExecutor> mCurrentTaskExecutor;
    stdsptr<_Task> mSelfRef;

    QList<stdptr<_Task>> mNextExecutionDependent;
    QList<stdptr<_Task>> mCurrentExecutionDependent;
};

class _ScheduledTask : public _Task {
public:
    _ScheduledTask() {
        mFinished = true;
    }
    ~_ScheduledTask();

    void beforeUpdate();

    virtual void schedulerProccessed();

    bool addScheduler();

    virtual bool shouldUpdate() {
        return true;
    }

    bool isAwaitingUpdate() { return mAwaitingUpdate; }
    bool schedulerAdded() { return mSchedulerAdded; }

    void clear();
    virtual bool isFileUpdatable() { return false; }

    virtual bool needsGpuProcessing() const { return false; }
protected:
    virtual void addSchedulerNow();
    bool mSchedulerAdded = false;
    bool mAwaitingUpdate = false;
};

#endif // UPDATABLE_H
