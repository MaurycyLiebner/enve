#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>
#include <QEventLoop>
#include "smartPointers/ememory.h"
#include "glhelpers.h"
#include "skia/skiaincludes.h"
class HddExecController;

enum class eTaskState {
    created,
    scheduled,
    qued,
    processing,
    finished,
    canceled,
    waiting
};

class eTask : public StdSelfRef {
    friend class TaskScheduler;
    friend class Que;
    template <typename T> friend class TaskCollection;
protected:
    eTask() {}

    virtual void scheduleTaskNow() = 0;
    virtual void afterQued() {}
    virtual void beforeProcessing(const Hardware) {}
    virtual void afterProcessing() {}
    virtual void afterCanceled() {}
public:
    ~eTask() { cancelDependent(); }

    struct Dependent {
        std::function<void()> fFinished;
        std::function<void()> fCanceled;
    };

    virtual HardwareSupport hardwareSupport() const = 0;
    virtual void processGpu(QGL33 * const gl,
                            SwitchableContext &context) = 0;
    virtual void process() = 0;

    virtual bool nextStep() { return false; }

    void taskQued() {
        mState = eTaskState::qued;
        afterQued();
    }

    bool scheduleTask();
    bool isQued() { return mState == eTaskState::qued; }
    bool isScheduled() { return mState == eTaskState::scheduled; }

    bool isActive() { return mState != eTaskState::created &&
                             mState != eTaskState::finished; }

    void aboutToProcess(const Hardware hw);
    void finishedProcessing();
    bool readyToBeProcessed();

    void addDependent(eTask * const updatable);
    void addDependent(const Dependent& func);

    bool finished();

    void decDependencies();
    void incDependencies();

    bool waitingToCancel() const { return mCancel; }

    void cancel() {
        if(mState == eTaskState::processing) {
            mCancel = true;
            return;
        }
        mState = eTaskState::canceled;
        cancelDependent();
        afterCanceled();
    }

    eTaskState getState() const {
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
    eTaskState mState = eTaskState::created;
private:
    void tellDependentThatFinished();
    void cancelDependent();

    bool mCancel = false;
    int mNDependancies = 0;
    QList<stdptr<eTask>> mDependent;
    QList<Dependent> mDependentF;
    std::exception_ptr mUpdateException;
};

class eCpuTask : public eTask {
    e_OBJECT
public:
    HardwareSupport hardwareSupport() const final {
        return HardwareSupport::cpuOnly;
    }

    void processGpu(QGL33 * const gl,
                    SwitchableContext &context) final {
        Q_UNUSED(gl);
        Q_UNUSED(context);
    }
protected:
    void scheduleTaskNow() final;
};

class eHddTask : public eTask {
    e_OBJECT
public:
    HardwareSupport hardwareSupport() const {
        return HardwareSupport::cpuOnly;
    }

    void processGpu(QGL33 * const gl,
                    SwitchableContext &context) {
        Q_UNUSED(gl);
        Q_UNUSED(context);
    }
protected:
    void scheduleTaskNow();
    void HddPartFinished();
public:
    void setController(HddExecController * const contr) {
        mController = contr;
    }
private:
    HddExecController * mController = nullptr;
};

class eCustomCpuTask : public eCpuTask {
    e_OBJECT
protected:
    eCustomCpuTask(const std::function<void(void)>& before,
                  const std::function<void(void)>& run,
                  const std::function<void(void)>& after) :
        mBefore(before), mRun(run), mAfter(after) {}

    void beforeProcessing(const Hardware) final {
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
class SPtrDisposer : public eCpuTask {
    e_OBJECT
protected:
    SPtrDisposer(const T& ptr) : mPtr(ptr) {}
public:
    void beforeProcessing(const Hardware) final {}
    void process() final { mPtr.reset(); }
    static eTask* sDispose(const T& ptr) {
        const auto disposer = enve::make_shared<SPtrDisposer<T>>(ptr);
        if(disposer->scheduleTask()) return disposer.get();
        return nullptr;
    }
protected:
    void afterProcessing() final {}
private:
    T mPtr;
};

#endif // UPDATABLE_H
