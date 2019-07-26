#include "effectsrenderer.h"
#include "PixmapEffects/rastereffects.h"
#include "GPUEffects/gpupostprocessor.h"
#include "boxrenderdata.h"

void EffectsRenderer::processGpu(QGL33 * const gl,
                                 SwitchableContext &context,
                                 BoxRenderData * const boxData) {
    Q_ASSERT(!mEffects.isEmpty());

    auto& srcImage = boxData->fRenderedImage;
    const int srcWidth = srcImage->width();
    const int srcHeight = srcImage->height();
    const QPoint gPos = boxData->fGlobalRect.topLeft();

    glViewport(0, 0, srcWidth, srcHeight);

    GpuRenderData renderData;
    renderData.fPosX = static_cast<int>(gPos.x());
    renderData.fPosY = static_cast<int>(gPos.y());
    renderData.fWidth = static_cast<uint>(srcWidth);
    renderData.fHeight = static_cast<uint>(srcHeight);
    auto& engine = renderData.fJSEngine;
    engine.evaluate("_texSize = [" + QString::number(srcWidth) + "," +
                    QString::number(srcHeight) + "]");
    engine.evaluate("_gPos = [" + QString::number(gPos.x()) + "," +
                    QString::number(gPos.y()) + "]");

    GpuRenderTools renderTools(gl, context, srcImage);
    while(!mEffects.isEmpty()) {
        const auto& effect = mEffects.first();
        if(effect->cpuOnly()) break;
        effect->processGpu(gl, renderTools, renderData);
        renderTools.swapTextures();
        mEffects.removeFirst();
    }

    boxData->fRenderedImage = renderTools.getSrcTexture().imageSnapshot(gl);
}

void EffectsRenderer::processCpu(BoxRenderData * const boxData) {
    auto& srcImage = boxData->fRenderedImage;
    const int srcWidth = srcImage->width();
    const int srcHeight = srcImage->height();
    const QPoint gPos = boxData->fGlobalRect.topLeft();
    srcImage = srcImage->makeRasterImage();
    Q_ASSERT(!mEffects.isEmpty());
    const auto& effect = mEffects.first();
    Q_ASSERT(!effect->gpuOnly());
    CpuRenderData renderData;
    renderData.fPosX = static_cast<int>(gPos.x());
    renderData.fPosY = static_cast<int>(gPos.y());
    renderData.fWidth = static_cast<uint>(srcWidth);
    renderData.fHeight = static_cast<uint>(srcHeight);
    SkPixmap pixmap;
    srcImage->peekPixels(&pixmap);
    SkBitmap bitmap;
    bitmap.installPixels(pixmap);
    CpuRenderTools renderTools(bitmap);
    effect->processCpu(renderTools, renderData);
    if(renderTools.hasBackupBitmap()) {
        srcImage = SkiaHelpers::transferDataToSkImage(
                    *const_cast<SkBitmap*>(&renderTools.fSrcDst));
    }
    mEffects.removeFirst();
}

void EffectsRenderer::setBaseGlobalRect(SkIRect &currRect,
                                        const SkIRect &skMaxBounds) const {
    for(const auto& effect : mEffects) {
        currRect = effect->setSrcRectUpdateDstRect(
                    currRect, skMaxBounds, true);
    }
}

HardwareSupport EffectsRenderer::nextHardwareSupport() const {
    Q_ASSERT(!isEmpty());
    return mEffects.first()->hardwareSupport();
}
