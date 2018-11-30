#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>
#include "selfref.h"
#include <QEventLoop>
#include "sharedpointerdefs.h"
class PaintControler;

class MinimalExecutor : public StdSelfRef {
public:
    MinimalExecutor();
    bool finished();

    virtual void addDependent(MinimalExecutor* updatable) = 0;

    virtual void clear();

    void decDependencies();
    void incDependencies();
protected:
    virtual void GUI_process();
    void tellDependentThatFinished();

    bool mGUIThreadExecution = false;
    bool mFinished = false;
    int nDependancies = 0;
    QList<MinimalExecutorPtr> mCurrentExecutionDependent;
};

class GUI_ThreadExecutor : public MinimalExecutor {
    GUI_ThreadExecutor();

    void addDependent(MinimalExecutor* updatable);
protected:
    void GUI_process() = 0;
};

class _Executor : public MinimalExecutor {
public:
    _Executor();

    void setCurrentPaintControler(PaintControler *paintControler);

    virtual void beforeUpdate();

    virtual void _processUpdate() = 0;

    void updateFinished();

    virtual void afterUpdate();

    bool isBeingProcessed();

    void waitTillProcessed();

    bool readyToBeProcessed();

    void clear();

    void addDependent(MinimalExecutor* updatable);
protected:
    bool mBeingProcessed = false;
    QPointer<PaintControler> mCurrentPaintControler;
    std::shared_ptr<_Executor> mSelfRef;

    QList<MinimalExecutorPtr> mNextExecutionDependent;
};

class _ScheduledExecutor : public _Executor {
public:
    _ScheduledExecutor() {
        mFinished = true;
    }
    ~_ScheduledExecutor();

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
protected:
    virtual void addSchedulerNow();
    bool mSchedulerAdded = false;
    bool mAwaitingUpdate = false;
};

#endif // UPDATABLE_H
