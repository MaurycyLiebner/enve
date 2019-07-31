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
#include "Tasks/updatable.h"

#include <QOpenGLFramebufferObject>
#include "exceptions.h"
class GpuPostProcessor : public QThread, protected OffscreenQGL33c {
    Q_OBJECT
public:
    GpuPostProcessor();
    void initialize();

    //! @brief Adds a new task and starts processing it if is not busy.
    void addToProcess(const stdsptr<Task>& scheduled) {
        //scheduled->afterProcessed(); return;
        Q_ASSERT(scheduled->gpuSupported());
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
    void afterProcessed();
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
            const auto grContext = GrContext::MakeGL(mInterface);
            if(!grContext) RuntimeThrow("Failed to make GrContext.");
            GLuint textureSquareVAO;
            iniTexturedVShaderVAO(this, textureSquareVAO);
            mInitialized = true;
            mContext.setContext(grContext, textureSquareVAO);

            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        for(const auto& scheduled : _mHandledProcesses) {
            try {
                scheduled->processGPU(this, mContext);
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
    SwitchableContext mContext;
    std::exception_ptr mProcessException;
    bool mFinished = true;
    bool mInitialized = false;
    GLuint _mTextureSquareVAO;
    QList<stdsptr<Task>> _mHandledProcesses;
    QList<stdsptr<Task>> mScheduledProcesses;
    //QOpenGLFramebufferObject* mFrameBuffer = nullptr;
};

#endif // GPUPOSTPROCESSOR_H
