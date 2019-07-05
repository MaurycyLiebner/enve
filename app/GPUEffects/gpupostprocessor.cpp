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
        const GLuint texturedSquareVAO) {
    if(!initializeOpenGLFunctions())
        RuntimeThrow("Initializing GL functions failed.");
    auto& srcImage = mBoxData->fRenderedImage;
    if(!srcImage) return;
    const int srcWidth = srcImage->width();
    const int srcHeight = srcImage->height();

    QJSEngine engine;
    engine.evaluate("_texSize = [" + QString::number(srcWidth) + "," +
                    QString::number(srcHeight) + "]");
    const QPointF gPos = mBoxData->fGlobalBoundingRect.topLeft();
    engine.evaluate("_gPos = [" + QString::number(gPos.x()) + "," +
                    QString::number(gPos.y()) + "]");

    glViewport(0, 0, srcWidth, srcHeight);

    GpuRenderTools renderTools(this, srcImage, texturedSquareVAO);
    auto& effects = mBoxData->fGPUEffects;
    const auto gPosX = static_cast<GLfloat>(gPos.x());
    const auto gPosY = static_cast<GLfloat>(gPos.y());
    for(const auto& effect : mBoxData->fGPUEffects) {
        effect->render(this, renderTools, engine, gPosX, gPosY);
        renderTools.swapTextures();
    }
    effects.clear();

    srcImage = renderTools.getSrcTexture().toImage(this);
}
