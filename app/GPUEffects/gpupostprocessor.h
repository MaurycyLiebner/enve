#ifndef GPUPOSTPROCESSOR_H
#define GPUPOSTPROCESSOR_H
#include <QOpenGLFramebufferObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include "offscreenqgl33c.h"
#include "smartPointers/stdselfref.h"
#include "skia/skiaincludes.h"
#include "glhelpers.h"
#include "GPUEffects/gpueffect.h"

class ScheduledPostProcess : public StdSelfRef, protected QGL33c {
    friend class GpuPostProcessor;
public:
    ScheduledPostProcess();
protected:
    virtual void afterProcessed() {}
    virtual void process(GrContext * const grContext,
                         const GLuint texturedSquareVAO) = 0;
private:
    bool unhandledException() const {
        return static_cast<bool>(mUpdateException);
    }

    void finishedProcessing() {
        afterProcessed();
        if(unhandledException()) {
            gPrintExceptionCritical(handleException());
        }
    }

    void setException(const std::exception_ptr& exception) {
        mUpdateException = exception;
    }

    std::exception_ptr handleException() {
        std::exception_ptr exc;
        mUpdateException.swap(exc);
        return exc;
    }

    std::exception_ptr mUpdateException;
};

class BoundingBoxRenderData;
class BoxRenderDataScheduledPostProcess : public ScheduledPostProcess {
public:
    BoxRenderDataScheduledPostProcess(
            const stdsptr<BoundingBoxRenderData> &boxData);
protected:
    void afterProcessed();
    void process(GrContext * const grContext,
                 const GLuint texturedSquareVAO);
private:
    const stdsptr<BoundingBoxRenderData> mBoxData;
};

#include <QOpenGLFramebufferObject>
#include "exceptions.h"
class GpuPostProcessor : public QThread, protected OffscreenQGL33c {
    Q_OBJECT
public:
    GpuPostProcessor();
    void initialize();

    //! @brief Adds a new task and starts processing it if is not busy.
    void addToProcess(const stdsptr<ScheduledPostProcess>& scheduled) {
        //scheduled->afterProcessed(); return;
        mScheduledProcesses << scheduled;
        handleScheduledProcesses();
    }

    void clear() {
        mScheduledProcesses.clear();
    }

    //! @brief Starts processing scheduled tasks if is not busy.
    void handleScheduledProcesses() {
        if(mScheduledProcesses.isEmpty()) return;
        if(!mFinished) return;
        mFinished = false;
        _mHandledProcesses << mScheduledProcesses.takeFirst();
        start();
    }

    //! @brief Returns true if nothing is waiting/being processed.
    bool hasFinished() const {
        return mFinished;
    }
signals:
    void processedAll();
private:
    void afterProcessed() {
        if(unhandledException())
            gPrintExceptionCritical(handleException());
        mFinished = true;
        for(const auto& process : _mHandledProcesses) {
            process->finishedProcessing();
        }
        _mHandledProcesses.clear();
        handleScheduledProcesses();
        if(mFinished) emit processedAll();
    }
protected:
    void run() override {
        try {
            processTasks();
        } catch(...) {
            setException(std::current_exception());
        }
    }

    void setException(const std::exception_ptr& exception) {
        mProcessException = exception;
    }

    void processTasks() {
        if(_mHandledProcesses.isEmpty()) return;
        makeCurrent();
        if(!mInitialized) {
            mInterface = GrGLMakeNativeInterface();
            if(!mInterface) RuntimeThrow("Failed to make native interface.");
            mGrContext = GrContext::MakeGL(mInterface);
            if(!mGrContext) RuntimeThrow("Failed to make GrContext.");

            iniTexturedVShaderVAO(this, _mTextureSquareVAO);
            mInitialized = true;

            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        for(const auto& scheduled : _mHandledProcesses) {
            try {
                scheduled->process(mGrContext.get(), _mTextureSquareVAO);
            } catch(...) {
                scheduled->setException(std::current_exception());
            }
        }
        doneCurrent();
        //mFrameBuffer->bindDefault();
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    bool unhandledException() const {
        return static_cast<bool>(mProcessException);
    }

    std::exception_ptr handleException() {
        std::exception_ptr exc;
        mProcessException.swap(exc);
        return exc;
    }

    sk_sp<const GrGLInterface> mInterface;
    sk_sp<GrContext> mGrContext;
    std::exception_ptr mProcessException;
    bool mFinished = true;
    bool mInitialized = false;
    GLuint _mTextureSquareVAO;
    QList<stdsptr<ScheduledPostProcess>> _mHandledProcesses;
    QList<stdsptr<ScheduledPostProcess>> mScheduledProcesses;
    //QOpenGLFramebufferObject* mFrameBuffer = nullptr;
};

#endif // GPUPOSTPROCESSOR_H
