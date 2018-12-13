#include "gpupostprocessor.h"
#include <QImage>
#include <QOpenGLTexture>
#include "skqtconversions.h"

GpuPostProcessor::GpuPostProcessor() {
    mOffscreenSurface = new QOffscreenSurface();
    mOffscreenSurface->create();
    mContext = new QOpenGLContext(mOffscreenSurface);
    mContext->setShareContext(QOpenGLContext::globalShareContext());
    MonoTry(mContext->create(), ContextCreateFailed);
}

ScheduledPostProcess::ScheduledPostProcess() {}

ShaderPostProcess::ShaderPostProcess(const sk_sp<SkImage> &srcImg,
                                     const stdsptr<ShaderProgramCallerBase> &program,
                                     const ShaderFinishedFunc &finishedFunc) :
    mProgram(program),
    mFinishedFunc(finishedFunc),
    mSrcImage(srcImg) {}

void ShaderPostProcess::process(const GLuint& texturedSquareVAO) {
//    mFinalImage = mSrcImage;
//    if(mFinishedFunc) mFinishedFunc(mFinalImage);
//    return;
    MonoTry(initializeOpenGLFunctions(), InitializeGLFuncsFailed);
    if(!mSrcImage) return;
    int srcWidth = mSrcImage->width();
    int srcHeight = mSrcImage->height();
    glViewport(0, 0, srcWidth, srcHeight);
    SkPixmap pix;
    mSrcImage->peekPixels(&pix);
    Texture srcTexture;
    srcTexture.gen(this, srcWidth, srcHeight, pix.addr());

    TextureFrameBuffer frameBufferObject;
    frameBufferObject.gen(this, srcWidth, srcHeight);

    glClear(GL_COLOR_BUFFER_BIT);
    mProgram->use(this);
    glActiveTexture(GL_TEXTURE0);
    srcTexture.bind(this);
    assertNoGlErrors();

    glBindVertexArray(texturedSquareVAO);
    assertNoGlErrors();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    assertNoGlErrors();
    frameBufferObject.bindTexture(this);

    mFinalImage = frameBufferObject.toImage();
    assertNoGlErrors();
    frameBufferObject.deleteFrameBuffer(this);
    frameBufferObject.deleteTexture(this);
    srcTexture.deleteTexture(this);
    if(mFinishedFunc) mFinishedFunc(mFinalImage);
}
