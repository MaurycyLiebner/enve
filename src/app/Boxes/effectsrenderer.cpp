#include "effectsrenderer.h"
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

#include "effectsubtaskspawner.h"
void EffectsRenderer::processCpu(BoxRenderData * const boxData) {
    Q_ASSERT(!mEffects.isEmpty());
    const auto& effect = mEffects.first();

    Q_ASSERT(!effect->gpuOnly());
    EffectSubTaskSpawner::sSpawn(effect, GetAsSPtr(boxData, BoxRenderData));
    mEffects.removeFirst();
}

void EffectsRenderer::setBaseGlobalRect(SkIRect &currRect,
                                        const SkIRect &baseRect,
                                        const SkIRect &skMaxBounds) const {
    for(const auto& effect : mEffects) {
        effect->setSrcRect(currRect, baseRect, skMaxBounds);
        currRect = effect->getDstRect();
    }
}

HardwareSupport EffectsRenderer::nextHardwareSupport() const {
    Q_ASSERT(!isEmpty());
    return mEffects.first()->hardwareSupport();
}
