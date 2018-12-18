#include "gpupostprocessor.h"
#include <QImage>
#include <QOpenGLTexture>
#include "skqtconversions.h"

GpuPostProcessor::GpuPostProcessor() {}

void GpuPostProcessor::initialize() {
    mOffscreenSurface = new QOffscreenSurface(nullptr, this);
    mOffscreenSurface->create();
    _mContext = new QOpenGLContext();
    _mContext->setShareContext(QOpenGLContext::globalShareContext());
    if(!_mContext->create()) {
        RuntimeThrow("Creating GL context failed.");
    }
    _mContext->moveToThread(this);
    connect(this, &GpuPostProcessor::finished,
            this, &GpuPostProcessor::afterProcessed);
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
    if(!initializeOpenGLFunctions()) {
        RuntimeThrow("Initializing GL functions failed.");
    }
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

#include "Boxes/boundingboxrenderdata.h"
BoxRenderDataScheduledPostProcess::BoxRenderDataScheduledPostProcess(
        const stdsptr<BoundingBoxRenderData>& boxData) : mBoxData(boxData) {
    assert(boxData->renderedToImage);
}

void BoxRenderDataScheduledPostProcess::afterProcessed() {
    mBoxData->updateFinished();
    mBoxData->fGpuFinished = false;
}

void BoxRenderDataScheduledPostProcess::process(
        const GLuint &texturedSquareVAO) {
    if(!initializeOpenGLFunctions()) {
        RuntimeThrow("Initializing GL functions failed.");
    }
    auto srcImage = mBoxData->renderedImage;
    if(!srcImage) return;
    int srcWidth = srcImage->width();
    int srcHeight = srcImage->height();
    glViewport(0, 0, srcWidth, srcHeight);
    SkPixmap pix;
    srcImage->peekPixels(&pix);
    Texture srcTexture;
    srcTexture.gen(this, srcWidth, srcHeight, pix.addr());

    TextureFrameBuffer frameBufferObject;
    frameBufferObject.gen(this, srcWidth, srcHeight);

    for(int i = 0; i < mBoxData->fGpuShaders.count(); i++) {
        const auto& program = mBoxData->fGpuShaders.at(i);
        glClear(GL_COLOR_BUFFER_BIT);
        program->use(this);
        glActiveTexture(GL_TEXTURE0);
        srcTexture.bind(this);
        assertNoGlErrors();

        glBindVertexArray(texturedSquareVAO);
        assertNoGlErrors();

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        assertNoGlErrors();
        if(i == mBoxData->fGpuShaders.count() - 1) break;
        frameBufferObject.swapTexture(this, srcTexture);
        assertNoGlErrors();
    }
    mBoxData->fGpuShaders.clear();

    frameBufferObject.bindTexture(this);
    mBoxData->renderedImage = frameBufferObject.toImage();
    assertNoGlErrors();
    frameBufferObject.deleteFrameBuffer(this);
    frameBufferObject.deleteTexture(this);
    srcTexture.deleteTexture(this);

    mBoxData->fGpuFinished = true;
}
