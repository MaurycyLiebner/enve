// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "effectsrenderer.h"
#include "Private/Tasks/gpupostprocessor.h"
#include "boxrenderdata.h"
#include "RasterEffects/rastereffectcaller.h"
#include "gpurendertools.h"

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
    engine.evaluate("eTexSize = [" + QString::number(srcWidth) + "," +
                                     QString::number(srcHeight) + "]");
    engine.evaluate("eGlobalPos = [" + QString::number(gPos.x()) + "," +
                                       QString::number(gPos.y()) + "]");

    GpuRenderTools renderTools(gl, context, srcImage);
    while(!mEffects.isEmpty()) {
        const auto& effect = mEffects.first();
        if(effect->hardwareSupport() == HardwareSupport::cpuOnly) break;
        effect->processGpu(gl, renderTools, renderData);
        mEffects.removeFirst();
    }

    boxData->fRenderedImage = renderTools.getSrcTexture().imageSnapshot(gl);
}

#include "effectsubtaskspawner.h"
void EffectsRenderer::processCpu(BoxRenderData * const boxData) {
    Q_ASSERT(!mEffects.isEmpty());
    const auto& effect = mEffects.first();

    Q_ASSERT(effect->hardwareSupport() != HardwareSupport::gpuOnly);
    EffectSubTaskSpawner::sSpawn(effect, boxData->ref<BoxRenderData>());
    mEffects.removeFirst();
}

void EffectsRenderer::setBaseGlobalRect(SkIRect &currRect,
                                        const SkIRect &skMaxBounds) const {
    for(const auto& effect : mEffects) {
        effect->setSrcRect(currRect, skMaxBounds);
        const auto dstRect = effect->getDstRect();
        currRect = SkIRect::MakeLTRB(qMin(dstRect.left(), currRect.left()),
                                     qMin(dstRect.top(), currRect.top()),
                                     qMax(dstRect.right(), currRect.right()),
                                     qMax(dstRect.bottom(), currRect.bottom()));
    }
}

HardwareSupport EffectsRenderer::nextHardwareSupport() const {
    Q_ASSERT(!isEmpty());
    return mEffects.first()->hardwareSupport();
}
