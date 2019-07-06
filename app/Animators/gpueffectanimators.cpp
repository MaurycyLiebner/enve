#include "gpueffectanimators.h"
#include "GPUEffects/gpurastereffect.h"
#include "Boxes/boundingbox.h"
#include <QDebug>

GPUEffectAnimators::GPUEffectAnimators(BoundingBox * const parentBox) :
    GPUEffectAnimatorsBase("gpu effects"), mParentBox_k(parentBox) {
    makeHiddenWhenEmpty();
}

QMarginsF GPUEffectAnimators::getEffectsMargin(const qreal relFrame) const {
    QMarginsF newMargin;
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect.get(), ShaderEffect);
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMarginAtRelFrame(relFrame);
        }
    }
    return newMargin;
}

void GPUEffectAnimators::addEffects(const qreal relFrame,
                                    BoundingBoxRenderData * const data) {
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect, ShaderEffect);
        if(pixmapEffect->isVisible()) {
            const auto effectRenderData =
                    pixmapEffect->getEffectCaller(relFrame);
            if(!effectRenderData) continue;
            data->fGPUEffects.append(effectRenderData);
        }
    }
}

void GPUEffectAnimators::updateIfUsesProgram(
        const ShaderEffectProgram * const program) {
    for(const auto& effect : ca_mChildAnimators) {
        if(!effect->SWT_isShaderEffect()) continue;
        const auto pixmapEffect = GetAsPtr(effect.get(), ShaderEffect);
        pixmapEffect->updateIfUsesProgram(program);
    }
}

qsptr<ShaderEffect> readIdCreateShaderEffect(QIODevice * const src) {
    const auto id = ShaderEffectCreator::sReadIdentifier(src);
    const auto best = ShaderEffectCreator::sGetBestCompatibleEffects(id);
    if(best.isEmpty()) RuntimeThrow("No compatible GPU effect found for " + id.fName);
    qsptr<ShaderEffect> effect;
    if(best.count() == 1) {
        const auto bestCreator = best.first();
        effect = GetAsSPtr(bestCreator->create(), ShaderEffect);
    } else {
        // exec ask dialog
    }
    return effect;
}

qsptr<GpuEffect> readIdCreateGPURasterEffect(QIODevice * const src) {
    GpuEffectType type;
    src->read(rcChar(&type), sizeof(GpuEffectType));
    switch(type) {
        case(GpuEffectType::BLUR):
            return nullptr;
        case(GpuEffectType::SHADOW):
            return nullptr;
        case(GpuEffectType::CUSTOM):
            return nullptr;
        case(GpuEffectType::CUSTOM_SHADER):
            return readIdCreateShaderEffect(src);
        default: RuntimeThrow("Invalid gpu effect type '" +
                              QString::number(int(type)) + "'");
    }
}

bool GPUEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}
