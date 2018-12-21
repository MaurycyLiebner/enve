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

    virtual void beforeProcessingStarted();

    virtual void _processUpdate() = 0;

    void finishedProcessing();

    bool isBeingProcessed();

    void waitTillProcessed();

    bool readyToBeProcessed();

    virtual void clear();

    virtual void addDependent(_Task* updatable);

    bool finished();

    void decDependencies();
    void incDependencies();
protected:
    virtual void afterProcessingFinished();
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

    void beforeProcessingStarted();

    virtual void taskQued();

    bool scheduleTask();

    virtual bool shouldUpdate() {
        return true;
    }

    bool isQued() { return mTaskQued; }
    bool isScheduled() { return mTaskScheduled; }

    void clear();
    virtual bool isHDDTask() { return false; }

    virtual bool needsGpuProcessing() const { return false; }
protected:
    virtual void scheduleTaskNow();
    bool mTaskScheduled = false;
    bool mTaskQued = false;
};

#endif // UPDATABLE_H
