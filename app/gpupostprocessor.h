#ifndef GPUPOSTPROCESSOR_H
#define GPUPOSTPROCESSOR_H
#include <QOpenGLFramebufferObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include "smartPointers/stdselfref.h"
#include "skiaincludes.h"
#include "glhelpers.h"


class ShaderProgramCallerBase : public StdSelfRef {
public:
    ShaderProgramCallerBase() {}
    virtual void use(QGL33c * const gl) = 0;
};

template<class T>
class ShaderProgramCaller : public ShaderProgramCallerBase {
    static_assert(std::is_base_of<ShaderProgram, T>::value,
                  "No valid ShaderProgram derived class associated with ShaderProgramCaller.");
public:
    ShaderProgramCaller(const T * const program) :
        mProgram(program) {}
protected:
    const T * const mProgram = nullptr;
};

class BlurProgramCaller : public ShaderProgramCaller<BlurProgram> {
public:
    BlurProgramCaller(const qreal& blurSize, const QSize& texSize) :
        ShaderProgramCaller(&GL_BLUR_PROGRAM) {
        mBlurRadiusX = static_cast<GLfloat>(blurSize/texSize.width());
        mBlurRadiusY = static_cast<GLfloat>(blurSize/texSize.height());
    }

    void use(QGL33c * const gl) {
        gl->glUseProgram(mProgram->fID);
        gl->glUniform2f(mProgram->fBlurRadiusLoc, mBlurRadiusX, mBlurRadiusY);
    }
private:
    GLfloat mBlurRadiusX;
    GLfloat mBlurRadiusY;
};

class ScheduledPostProcess : public StdSelfRef,
        protected QGL33c {
    friend class GpuPostProcessor;
    friend class ComplexScheduledPostProcess;
public:
    ScheduledPostProcess();
private:
    virtual void process(const GLuint &texturedSquareVAO) = 0;
};

typedef std::function<void(sk_sp<SkImage>)> ShaderFinishedFunc;
class ShaderPostProcess : public ScheduledPostProcess {
public:
    ShaderPostProcess(const sk_sp<SkImage>& srcImg,
                      const stdsptr<ShaderProgramCallerBase> &program,
                      const ShaderFinishedFunc& finishedFunc = ShaderFinishedFunc());
private:
    const stdsptr<ShaderProgramCallerBase> mProgram;
    //! @brief Gets called after processing finished, provides resulting image.
    const ShaderFinishedFunc mFinishedFunc;
    sk_sp<SkImage> mSrcImage;
    sk_sp<SkImage> mFinalImage;

    //! @brief Uses shaders to draw the source image to the final texture.
    void process(const GLuint &texturedSquareVAO);
};

class ComplexScheduledPostProcess : public ScheduledPostProcess {
public:
    ComplexScheduledPostProcess();

private:
    void process(const GLuint &texturedSquareVAO) {
        foreach(const auto& child, mChildProcesses) {
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

    void addToProcess(const stdsptr<ScheduledPostProcess>& scheduled) {
        mScheduledProcesses << scheduled;
    }

    void clear() {
        mScheduledProcesses.clear();
    }

    void handleScheduledProcesses() {
        if(mScheduledProcesses.isEmpty()) return;
        if(isRunning()) return;
        _mHandledProcesses = mScheduledProcesses;
        mScheduledProcesses.clear();
        start();
    }
signals:
    void runFinished();
private slots:
    void finishedProcessing() {
        handleScheduledProcesses();
    }
protected:
    void _process() {
        if(_mHandledProcesses.isEmpty()) return;
        MonoTry(_mContext->makeCurrent(mOffscreenSurface), ContextCurrentFailed);
        if(!_mInitialized) {
            MonoTry(initializeOpenGLFunctions(), InitializeGLFuncsFailed);
            iniTexturedVShaderVAO(this, _mTextureSquareVAO);
            _mInitialized = true;

            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        assertNoGlErrors();

        foreach(const auto& scheduled, _mHandledProcesses) {
            scheduled->process(_mTextureSquareVAO);
            assertNoGlErrors();
        }
        _mHandledProcesses.clear();
        _mContext->doneCurrent();
        //mFrameBuffer->bindDefault();
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void run() override {
        _process();

        emit runFinished();
    }

    bool _mInitialized = false;
    GLuint _mTextureSquareVAO;
    QOpenGLContext* _mContext = nullptr;
    QList<stdsptr<ScheduledPostProcess>> _mHandledProcesses;

    QList<stdsptr<ScheduledPostProcess>> mScheduledProcesses;
    QOffscreenSurface *mOffscreenSurface = nullptr;
    //QOpenGLFramebufferObject* mFrameBuffer = nullptr;
};

#endif // GPUPOSTPROCESSOR_H
