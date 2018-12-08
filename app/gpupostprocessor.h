#ifndef GPUPOSTPROCESSOR_H
#define GPUPOSTPROCESSOR_H
#include "smartPointers/stdselfref.h"
#include "skiaincludes.h"
#include <QOpenGLFunctions_3_0>
#include "texvertexdata.h"

class ScheduledPostProcess : public StdSelfRef,
        protected QOpenGLFunctions_3_0 {
    friend class GpuPostProcessor;
    friend class ComplexScheduledPostProcess;
public:
    ScheduledPostProcess();
private:
    virtual void process(GrContext * const grContext) = 0;
};

class ShaderPostProcess : public ScheduledPostProcess {
public:
    ShaderPostProcess();
private:
    GLuint mProgram;
    sk_sp<SkImage> mSrcImage;
    sk_sp<SkImage> mFinalImage;

    void generateFinalTexture(const int& finalWidth,
                              const int& finalHeight,
                              GrGLTextureInfo& finalTexInfo) {
        finalTexInfo.fFormat = GL_RGBA;
        finalTexInfo.fTarget = GL_TEXTURE_2D;

        // create a color attachment texture
        glGenTextures(1, &finalTexInfo.fID);
        glBindTexture(GL_TEXTURE_2D, finalTexInfo.fID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, finalWidth, finalHeight,
                     0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, finalTexInfo.fID, 0);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            qDebug() << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    }

    void process(GrContext * const grContext) {
        int margin = 0;
        int srcWidth = mSrcImage->width();
        int srcHeight = mSrcImage->height();
        int finalWidth = srcWidth + 2*margin;
        int finalHeight = srcHeight + 2*margin;
        GrGLTextureInfo finalTexInfo;
        generateFinalTexture(finalWidth, finalHeight, finalTexInfo);

        glUseProgram(mProgram);
        glViewport(margin, margin, srcWidth, srcHeight);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(MY_GL_VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        GrBackendTexture finalTex(finalWidth, finalHeight,
                                  GrMipMapped::kNo, finalTexInfo);
        mFinalImage = SkImage::MakeFromAdoptedTexture(grContext, finalTex,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      kBGRA_8888_SkColorType,
                                                      kPremul_SkAlphaType);
    }
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

class GpuPostProcessor : protected QOpenGLFunctions_3_0 {
public:
    GpuPostProcessor();

    void process() {
        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
protected:
    GLuint mFrameBufferId;
};

#endif // GPUPOSTPROCESSOR_H
