#ifndef GPUPOSTPROCESSOR_H
#define GPUPOSTPROCESSOR_H
#include <QOpenGLFramebufferObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include "smartPointers/stdselfref.h"
#include "skiaincludes.h"
#include "glhelpers.h"

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
                      const GLuint &program,
                      const ShaderFinishedFunc& finishedFunc = ShaderFinishedFunc());
private:
    const GLuint mProgram;
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
class GpuPostProcessor : protected QGL33c {
public:
    GpuPostProcessor();

    void process() {
        if(mScheduledProcesses.isEmpty()) return;
        Q_ASSERT(mContext->makeCurrent(mOffscreenSurface));
        if(!mInitialized) {
            Q_ASSERT(initializeOpenGLFunctions());
            iniTexturedVShaderVAO(this, mTextureSquareVAO);
            mInitialized = true;

            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        assertNoGlErrors();

        foreach(const auto& scheduled, mScheduledProcesses) {
            scheduled->process(mTextureSquareVAO);
            assertNoGlErrors();
        }
        mScheduledProcesses.clear();
        mContext->doneCurrent();
        //mFrameBuffer->bindDefault();
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void addToProcess(const stdsptr<ScheduledPostProcess>& scheduled) {
        mScheduledProcesses << scheduled;
    }

    void clear() {
        mScheduledProcesses.clear();
    }
protected:
    bool mInitialized = false;
    GLuint mTextureSquareVAO;

    QOffscreenSurface *mOffscreenSurface = nullptr;
    QOpenGLContext* mContext = nullptr;
    //QOpenGLFramebufferObject* mFrameBuffer = nullptr;
    QList<stdsptr<ScheduledPostProcess>> mScheduledProcesses;
};

#endif // GPUPOSTPROCESSOR_H
