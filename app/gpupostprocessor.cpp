#include "gpupostprocessor.h"

GpuPostProcessor::GpuPostProcessor() {}

ScheduledPostProcess::ScheduledPostProcess() {
    Q_ASSERT(initializeOpenGLFunctions());
}

ShaderPostProcess::ShaderPostProcess(const sk_sp<SkImage> &srcImg,
                                     const GLuint &program,
                                     const ShaderFinishedFunc &finishedFunc) :
    mProgram(program),
    mFinishedFunc(finishedFunc),
    mSrcImage(srcImg) {}

void ShaderPostProcess::generateFinalTexture(const int &finalWidth,
                                             const int &finalHeight,
                                             GrGLTextureInfo &finalTexInfo) {
    finalTexInfo.fFormat = GL_BGRA;
    finalTexInfo.fTarget = GL_TEXTURE_2D;

    // create a color attachment texture
    glGenTextures(1, &finalTexInfo.fID);
    glBindTexture(GL_TEXTURE_2D, finalTexInfo.fID);
    assertNoGlErrors();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, finalWidth, finalHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    assertNoGlErrors();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    assertNoGlErrors();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, finalTexInfo.fID, 0);
    assertNoGlErrors();
    auto frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(frameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
        qDebug() << frameBufferStatus << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
}

void ShaderPostProcess::adoptTexture(GrContext * const grContext,
                                     const int &finalWidth,
                                     const int &finalHeight,
                                     const GrGLTextureInfo &finalTexInfo) {
    GrBackendTexture finalTex(finalWidth, finalHeight,
                              GrMipMapped::kNo, finalTexInfo);
    mFinalImage = SkImage::MakeFromAdoptedTexture(grContext, finalTex,
                                                  kTopLeft_GrSurfaceOrigin,
                                                  kBGRA_8888_SkColorType,
                                                  kPremul_SkAlphaType);
}

void ShaderPostProcess::process(GrContext * const grContext,
                                const GLuint& texturedSquareVAO) {
    if(!mSrcImage) return;
    GLuint srcTexID;
    sk_sp<SkImage> srcTexture(mSrcImage->makeTextureImage(grContext, nullptr));
    if(!srcTexture) return;
    if(!srcTexture->isValid(grContext)) return;
    GrGLTextureInfo glTexInfo;
    auto backendTex = srcTexture->getBackendTexture(false);
    Q_ASSERT(backendTex.isValid());
    backendTex.getGLTextureInfo(&glTexInfo);
    srcTexID = glTexInfo.fID;

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
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexID);
    glBindVertexArray(texturedSquareVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    assertNoGlErrors();

    adoptTexture(grContext, finalWidth, finalHeight, finalTexInfo);
    if(mFinishedFunc) mFinishedFunc(mFinalImage);
}
