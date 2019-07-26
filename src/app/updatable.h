#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>
#include <QEventLoop>
#include "smartPointers/sharedpointerdefs.h"
#include "glhelpers.h"
#include "skia/skiaincludes.h"
class HDDExecController;

class Que;

class Task : public StdSelfRef {
    friend class TaskScheduler;
    friend class Que;
    template <typename T> friend class TaskCollection;
protected:
    Task() {}

    virtual void scheduleTaskNow() = 0;
    virtual void afterQued() {}
    virtual void beforeProcessing() {}
    virtual void afterProcessing() {}
    virtual void afterCanceled() {}
public:
    ~Task() { cancelDependent(); }

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

    virtual HardwareSupport hardwareSupport() const = 0;
    virtual void processGPU(QGL33 * const gl,
                            SwitchableContext &context) = 0;
    virtual void process() = 0;

    virtual bool nextStep() { return false; }

    bool cpuSupported() const { return hardwareSupport() != HardwareSupport::GPU_ONLY; }
    bool cpuPreferred() const { return hardwareSupport() == HardwareSupport::CPU_PREFFERED; }
    bool cpuOnly() const { return hardwareSupport() == HardwareSupport::CPU_ONLY; }

    bool gpuSupported() const { return hardwareSupport() != HardwareSupport::CPU_ONLY; }
    bool gpuPreferred() const { return hardwareSupport() == HardwareSupport::GPU_PREFFERED; }
    bool gpuOnly() const { return hardwareSupport() == HardwareSupport::GPU_ONLY; }

    void taskQued() {
        mState = QUED;
        afterQued();
    }

    bool scheduleTask();
    bool isQued() { return mState == QUED; }
    bool isScheduled() { return mState == SCHEDULED; }

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
public:
    HardwareSupport hardwareSupport() const {
        return HardwareSupport::CPU_ONLY;
    }

    void processGPU(QGL33 * const gl,
                    SwitchableContext &context) {
        Q_UNUSED(gl);
        Q_UNUSED(context);
    }
protected:
    void scheduleTaskNow() final;
};

class HDDTask : public Task {
    friend class StdSelfRef;
public:
    HardwareSupport hardwareSupport() const {
        return HardwareSupport::CPU_ONLY;
    }

    void processGPU(QGL33 * const gl,
                    SwitchableContext &context) {
        Q_UNUSED(gl);
        Q_UNUSED(context);
    }
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

class CustomCPUTask : public CPUTask {
    friend class StdSelfRef;
protected:
    CustomCPUTask(const std::function<void(void)>& before,
                  const std::function<void(void)>& run,
                  const std::function<void(void)>& after) :
        mBefore(before), mRun(run), mAfter(after) {}

    void beforeProcessing() final {
        if(mBefore) mBefore();
    }

    void afterProcessing() final {
        if(mAfter) mAfter();
    }

    void process() final { if(mRun) mRun(); }
private:
    const std::function<void(void)> mBefore;
    const std::function<void(void)> mRun;
    const std::function<void(void)> mAfter;
};

template <typename T>
class SPtrDisposer : public CPUTask {
    friend class StdSelfRef;
protected:
    SPtrDisposer(const T& ptr) : mPtr(ptr) {}
public:
    void beforeProcessing() final {}
    void process() final { mPtr.reset(); }
    static Task* sDispose(const T& ptr) {
        const auto disposer = SPtrCreateTemplated(SPtrDisposer<T>)(ptr);
        if(disposer->scheduleTask()) return disposer.get();
        return nullptr;
    }
protected:
    void afterProcessing() final {}
private:
    T mPtr;
};

#endif // UPDATABLE_H
