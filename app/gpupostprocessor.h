#ifndef GPUPOSTPROCESSOR_H
#define GPUPOSTPROCESSOR_H
#include "smartPointers/stdselfref.h"
#include "skiaincludes.h"
#include <QOpenGLFunctions_3_3_Core>
#include "texvertexdata.h"

class ScheduledPostProcess : public StdSelfRef,
        protected QOpenGLFunctions_3_3_Core {
    friend class GpuPostProcessor;
    friend class ComplexScheduledPostProcess;
public:
    ScheduledPostProcess();
private:
    virtual void process(GrContext * const grContext) = 0;
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

    //! @brief Generates the texture processor will render to.
    void generateFinalTexture(const int& finalWidth,
                              const int& finalHeight,
                              GrGLTextureInfo& finalTexInfo);

    //! @brief Wraps the resulting texture, called when finished processing.
    void adoptTexture(GrContext * const grContext,
                      const int& finalWidth,
                      const int& finalHeight,
                      const GrGLTextureInfo& finalTexInfo);

    //! @brief Uses shaders to draw the source image to the final texture.
    void process(GrContext * const grContext);
};

class ComplexScheduledPostProcess : public ScheduledPostProcess {
public:
    ComplexScheduledPostProcess();

private:
    void process(GrContext * const grContext) {
        foreach(const auto& child, mChildProcesses) {
            child->process(grContext);
        }
    }
    QList<stdsptr<ScheduledPostProcess>> mChildProcesses;
};

class GpuPostProcessor : protected QOpenGLFunctions_3_3_Core {
public:
    GpuPostProcessor();

    void process(GrContext * const grContext) {
        if(mScheduledProcesses.isEmpty()) return;
        if(!mFrameBufferCreated) {
            glGenFramebuffers(1, &mFrameBufferId);
            mFrameBufferCreated = true;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);

        foreach(const auto& scheduled, mScheduledProcesses) {
            scheduled->process(grContext);
        }
        mScheduledProcesses.clear();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void addToProcess(const stdsptr<ScheduledPostProcess>& scheduled) {
        mScheduledProcesses << scheduled;
    }

    void clear() {
        mScheduledProcesses.clear();
    }
protected:
    bool mFrameBufferCreated = false;
    GLuint mFrameBufferId;
    QList<stdsptr<ScheduledPostProcess>> mScheduledProcesses;
};

#endif // GPUPOSTPROCESSOR_H
