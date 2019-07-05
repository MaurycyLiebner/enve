#include "gpupostprocessor.h"
#include <QImage>
#include <QOpenGLTexture>
#include "skia/skqtconversions.h"

GpuPostProcessor::GpuPostProcessor() {
    connect(this, &QThread::finished,
            this, &GpuPostProcessor::afterProcessed);
}

void GpuPostProcessor::initialize() {
    OffscreenQGL33c::initialize();
    moveContextToThread(this);
}

ScheduledPostProcess::ScheduledPostProcess() {}

ShaderPostProcess::ShaderPostProcess(const sk_sp<SkImage> &srcImg,
                                     const stdsptr<GPURasterEffectCaller> &program,
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
    QJSEngine engine;
    mProgram->use(this, engine);
    glActiveTexture(GL_TEXTURE0);
    srcTexture.bind(this);

    glBindVertexArray(texturedSquareVAO);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    frameBufferObject.bindTexture(this);

    mFinalImage = frameBufferObject.toImage();
    frameBufferObject.deleteFrameBuffer(this);
    frameBufferObject.deleteTexture(this);
    srcTexture.deleteTexture(this);
    if(mFinishedFunc) mFinishedFunc(mFinalImage);
}

#include "Boxes/boundingboxrenderdata.h"
BoxRenderDataScheduledPostProcess::BoxRenderDataScheduledPostProcess(
        const stdsptr<BoundingBoxRenderData>& boxData) : mBoxData(boxData) {}

void BoxRenderDataScheduledPostProcess::afterProcessed() {
    mBoxData->finishedProcessing();
}

void BoxRenderDataScheduledPostProcess::process(
        const GLuint &texturedSquareVAO) {
    if(!initializeOpenGLFunctions()) {
        RuntimeThrow("Initializing GL functions failed.");
    }
    auto srcImage = mBoxData->fRenderedImage;
    if(!srcImage) return;
    int srcWidth = srcImage->width();
    int srcHeight = srcImage->height();

    QJSEngine engine;
    engine.evaluate("_texSize = [" + QString::number(srcWidth) + "," +
                    QString::number(srcHeight) + "]");
    const QPointF gPos = mBoxData->fGlobalBoundingRect.topLeft();
    engine.evaluate("_gPos = [" + QString::number(gPos.x()) + "," +
                    QString::number(gPos.y()) + "]");

    glViewport(0, 0, srcWidth, srcHeight);
    SkPixmap pix;
    srcImage->peekPixels(&pix);
    Texture srcTexture;
    srcTexture.gen(this, srcWidth, srcHeight, pix.addr());

    TextureFrameBuffer frameBufferObject;
    frameBufferObject.gen(this, srcWidth, srcHeight);

    for(int i = 0; i < mBoxData->fGPUEffects.count(); i++) {
        if(i != 0) frameBufferObject.swapTexture(this, srcTexture);

        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        srcTexture.bind(this);

        const auto& program = mBoxData->fGPUEffects.at(i);
        program->setGlobalPos(gPos);
        program->use(this, engine);

        glBindVertexArray(texturedSquareVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    }
    mBoxData->fGPUEffects.clear();

    frameBufferObject.bindTexture(this);
    mBoxData->fRenderedImage = frameBufferObject.toImage();
    frameBufferObject.deleteFrameBuffer(this);
    frameBufferObject.deleteTexture(this);
    srcTexture.deleteTexture(this);
}
