#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>
#include <QEventLoop>
#include "smartPointers/sharedpointerdefs.h"
class HDDExecController;

class Que;

class Task : public StdSelfRef {
    friend class TaskScheduler;
    friend class Que;
protected:
    Task() {}

    virtual void scheduleTaskNow() = 0;
    virtual void beforeProcessing() {}
    virtual void afterProcessing() {}
    virtual void afterCanceled() {}
public:
    struct Dependent {
        std::function<void()> fFinished;
        std::function<void()> fCanceled;
    };

    enum State : char {
        CREATED,
        SCHEDULED,
        QUED,
        PROCESSING,
        FINISHED,
        CANCELED,
        FAILED
    };

    enum GpuSupport {
        GPU_NO_SUPPORT,
        GPU_PREFERRED,
        GPU_REQUIRED
    };

    virtual void processTask() = 0;
    virtual bool gpuProcessingNeeded() const { return false; }
    virtual GpuSupport gpuSupport() const { return GPU_NO_SUPPORT; }
    bool gpuProcessingSupported() const { return gpuSupport() > GPU_NO_SUPPORT; }
    bool gpuProcessingPreferred() const { return gpuSupport() == GPU_PREFERRED; }
    bool gpuProcessingOnly() const { return gpuSupport() == GPU_REQUIRED; }
    virtual void taskQued() { mState = QUED; }

    bool scheduleTask();
    bool isQued() { return mState == QUED; }
    bool isScheduled() { return mState == SCHEDULED; }

    ~Task() { cancelDependent(); }

    bool isActive() { return mState != CREATED && mState != FINISHED; }

    void aboutToProcess();
    void finishedProcessing();
    bool readyToBeProcessed();

    void addDependent(Task * const updatable);
    void addDependent(const Dependent& func);

    bool finished();

    void decDependencies();
    void incDependencies();

    void cancel() {
        mState = CANCELED;
        cancelDependent();
        afterCanceled();
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
    void cancelDependent();

    int mNDependancies = 0;
    QList<stdptr<Task>> mDependent;
    QList<Dependent> mDependentF;
    std::exception_ptr mUpdateException;
};

class CPUTask : public Task {
    friend class StdSelfRef;
protected:
    void scheduleTaskNow() final;
};

class HDDTask : public Task {
    friend class StdSelfRef;
protected:
    void scheduleTaskNow();
    void HDDPartFinished();
public:
    void setController(HDDExecController * const contr) {
        mController = contr;
    }
private:
    HDDExecController * mController = nullptr;
};

class StdSPtrDisposer : public CPUTask {
    friend class StdSelfRef;
protected:
    StdSPtrDisposer(const stdsptr<void>& ptr) : mPtr(ptr) {}
public:
    void beforeProcessing() final {}
    void processTask() final { mPtr.reset(); }
protected:
    void afterProcessing() final {}
private:
    stdsptr<void> mPtr;
};
#include "skia/skiaincludes.h"
class SkImageSPtrDisposer : public CPUTask {
    friend class StdSelfRef;
protected:
    SkImageSPtrDisposer(const sk_sp<SkImage>& ptr) : mPtr(ptr) {}
public:
    void beforeProcessing() final {}
    void processTask() final { mPtr.reset(); }
protected:
    void afterProcessing() final {}
private:
    sk_sp<SkImage> mPtr;
};

class QSPtrDisposer : public CPUTask {
    friend class StdSelfRef;
protected:
    QSPtrDisposer(const qsptr<QObject>& ptr) : mPtr(ptr) {}
public:
    void beforeProcessing() final {}
    void processTask() final { mPtr.reset(); }
protected:
    void afterProcessing() final {}
private:
    qsptr<QObject> mPtr;
};

class CustomCPUTask : public CPUTask {
    friend class StdSelfRef;
protected:
    CustomCPUTask(const std::function<void(void)>& before,
                  const std::function<void(void)>& run,
                  const std::function<void(void)>& after) :
        mBefore(before), mRun(run), mAfter(after) {}
public:
    void beforeProcessing() final {
        if(mBefore) mBefore();
    }

    void processTask() final {
        if(mRun) mRun();
    }

protected:
    void afterProcessing() final {
        if(mAfter) mAfter();
    }
private:
    const std::function<void(void)> mBefore;
    const std::function<void(void)> mRun;
    const std::function<void(void)> mAfter;
};

class CustomHDDTask : public HDDTask {
    friend class StdSelfRef;
public:
    void beforeProcessing() final {
        if(mBefore) mBefore();
    }

    void processTask() final {
        if(mRun) mRun();
    }

protected:
    void afterProcessing() final {
        if(mAfter) mAfter();
    }

    CustomHDDTask(const std::function<void(void)>& before,
                  const std::function<void(void)>& run,
                  const std::function<void(void)>& after) :
        mBefore(before), mRun(run), mAfter(after) {}
private:
    const std::function<void(void)> mBefore;
    const std::function<void(void)> mRun;
    const std::function<void(void)> mAfter;
};

#endif // UPDATABLE_H
