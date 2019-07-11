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

#include "Boxes/boundingboxrenderdata.h"
BoxRenderDataScheduledPostProcess::BoxRenderDataScheduledPostProcess(
        const stdsptr<BoundingBoxRenderData>& boxData) : mBoxData(boxData) {}

void BoxRenderDataScheduledPostProcess::afterProcessed() {
    mBoxData->finishedProcessing();
}

void BoxRenderDataScheduledPostProcess::process(
        GrContext* const grContext,
        const GLuint texturedSquareVAO) {
    if(!initializeOpenGLFunctions())
        RuntimeThrow("Initializing GL functions failed.");
    auto& srcImage = mBoxData->fRenderedImage;
    if(!srcImage) mBoxData->processTaskWithGPU(this, grContext);
    if(!mBoxData->fRenderedImage) return;
    if(mBoxData->fGPUEffects.isEmpty()) {
        srcImage = srcImage->makeRasterImage();
        return;
    }
    const int srcWidth = srcImage->width();
    const int srcHeight = srcImage->height();
    const QPointF gPos = mBoxData->fGlobalBoundingRect.topLeft();

    glViewport(0, 0, srcWidth, srcHeight);

    GpuRenderData renderData;
    renderData.fPosX = static_cast<GLfloat>(gPos.x());
    renderData.fPosY = static_cast<GLfloat>(gPos.y());
    renderData.fWidth = static_cast<GLuint>(srcWidth);
    renderData.fHeight = static_cast<GLuint>(srcHeight);
    auto& engine = renderData.fJSEngine;
    engine.evaluate("_texSize = [" + QString::number(srcWidth) + "," +
                    QString::number(srcHeight) + "]");
    engine.evaluate("_gPos = [" + QString::number(gPos.x()) + "," +
                    QString::number(gPos.y()) + "]");

    GpuRenderTools renderTools(this, grContext,
                               srcImage, texturedSquareVAO);
    auto& effects = mBoxData->fGPUEffects;
    for(const auto& effect : mBoxData->fGPUEffects) {
        effect->render(this, renderTools, renderData);
        renderTools.swapTextures();
    }
    effects.clear();

    srcImage = renderTools.getSrcTexture().toImage(this);
}
