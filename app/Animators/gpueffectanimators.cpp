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
        const auto pixmapEffect = GetAsPtr(effect.get(), ShaderEffect);
        pixmapEffect->updateIfUsesProgram(program);
    }
}

qsptr<ShaderEffect> readIdCreateGPURasterEffect(QIODevice * const src) {
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

bool GPUEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}
