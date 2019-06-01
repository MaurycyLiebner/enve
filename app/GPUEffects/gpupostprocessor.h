#ifndef GPUPOSTPROCESSOR_H
#define GPUPOSTPROCESSOR_H
#include <QOpenGLFramebufferObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include "smartPointers/stdselfref.h"
#include "skia/skiaincludes.h"
#include "glhelpers.h"
#include "gpurastereffect.h"

class ScheduledPostProcess : public StdSelfRef,
        protected QGL33c {
    friend class GpuPostProcessor;
    friend class ComplexScheduledPostProcess;
    friend class BoxRenderDataScheduledPostProcess;
public:
    ScheduledPostProcess();
    void finishedProcessing() {
        afterProcessed();
        if(unhandledException()) {
            gPrintExceptionCritical(handleException());
        }
    }

    void setException(const std::exception_ptr& exception) {
        mUpdateException = exception;
    }

protected:
    virtual void afterProcessed() {}
private:
    virtual void process(const GLuint &texturedSquareVAO) = 0;

    bool unhandledException() const {
        return static_cast<bool>(mUpdateException);
    }

    std::exception_ptr handleException() {
        std::exception_ptr exc;
        mUpdateException.swap(exc);
        return exc;
    }

    std::exception_ptr mUpdateException;
};

typedef std::function<void(sk_sp<SkImage>)> ShaderFinishedFunc;
class ShaderPostProcess : public ScheduledPostProcess {
public:
    ShaderPostProcess(const sk_sp<SkImage>& srcImg,
                      const stdsptr<GPURasterEffectCaller> &program,
                      const ShaderFinishedFunc& finishedFunc = ShaderFinishedFunc());
private:
    const stdsptr<GPURasterEffectCaller> mProgram;
    //! @brief Gets called after processing finished, provides resulting image.
    const ShaderFinishedFunc mFinishedFunc;
    sk_sp<SkImage> mSrcImage;
    sk_sp<SkImage> mFinalImage;

    //! @brief Uses shaders to draw the source image to the final texture.
    void process(const GLuint &texturedSquareVAO);
};
class BoundingBoxRenderData;
class BoxRenderDataScheduledPostProcess : public ScheduledPostProcess {
public:
    BoxRenderDataScheduledPostProcess(
            const stdsptr<BoundingBoxRenderData> &boxData);
protected:
    void afterProcessed();
private:
    void process(const GLuint &texturedSquareVAO);
    const stdsptr<BoundingBoxRenderData> mBoxData;
};

class ComplexScheduledPostProcess : public ScheduledPostProcess {
public:
    ComplexScheduledPostProcess();

private:
    void process(const GLuint &texturedSquareVAO) {
        for(const auto& child : mChildProcesses) {
            child->process(texturedSquareVAO);
        }
    }
    QList<stdsptr<ScheduledPostProcess>> mChildProcesses;
};
#include <QOpenGLFramebufferObject>
#include "exceptions.h"
class GpuPostProcessor : public QThread, protected QGL33c {
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
private slots:
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
        if(!_mContext->makeCurrent(mOffscreenSurface)) {
            RuntimeThrow("Making GL context current failed.");
        }
        if(!_mInitialized) {
            if(!initializeOpenGLFunctions()) {
                RuntimeThrow("Initializing GL functions failed.");
            }
            iniTexturedVShaderVAO(this, _mTextureSquareVAO);
            _mInitialized = true;

            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        for(const auto& scheduled : _mHandledProcesses) {
            try {
                scheduled->process(_mTextureSquareVAO);
            } catch(...) {
                scheduled->setException(std::current_exception());
            }
        }
        _mContext->doneCurrent();
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

    std::exception_ptr mProcessException;
    bool mFinished = true;
    bool _mInitialized = false;
    GLuint _mTextureSquareVAO;
    QOpenGLContext* _mContext = nullptr;
    QList<stdsptr<ScheduledPostProcess>> _mHandledProcesses;

    QList<stdsptr<ScheduledPostProcess>> mScheduledProcesses;
    QOffscreenSurface *mOffscreenSurface = nullptr;
    //QOpenGLFramebufferObject* mFrameBuffer = nullptr;
};

#endif // GPUPOSTPROCESSOR_H
