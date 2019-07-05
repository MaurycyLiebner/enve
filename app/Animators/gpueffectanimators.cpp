#include "gpueffectanimators.h"
#include "GPUEffects/gpurastereffect.h"
#include "Boxes/boundingbox.h"
#include <QDebug>

GPUEffectAnimators::GPUEffectAnimators(BoundingBox *parentBox) :
    GPUEffectAnimatorsBase("gpu effects"), mParentBox_k(parentBox) {
    makeHiddenWhenEmpty();
}

QMarginsF GPUEffectAnimators::getEffectsMargin(const qreal relFrame) const {
    QMarginsF newMargin;
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect.get(), GPURasterEffect);
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMarginAtRelFrame(relFrame);
        }
    }
    return newMargin;
}

void GPUEffectAnimators::addEffectRenderDataToListF(
        const qreal relFrame,
        BoundingBoxRenderData * const data) {
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect, GPURasterEffect);
        if(pixmapEffect->isVisible()) {
            const auto effectRenderData =
                    pixmapEffect->getGPURasterEffectCaller(relFrame);
            if(!effectRenderData) continue;
            data->fGPUEffects.append(effectRenderData);
        }
    }
}

void GPUEffectAnimators::updateIfUsesProgram(
        const GPURasterEffectProgram * const program) {
    for(const auto& effect : ca_mChildAnimators) {
        const auto pixmapEffect = GetAsPtr(effect.get(), GPURasterEffect);
        pixmapEffect->updateIfUsesProgram(program);
    }
}

qsptr<GPURasterEffect> readIdCreateGPURasterEffect(QIODevice * const src) {
    const auto id = GPURasterEffectCreator::sReadIdentifier(src);
    const auto best = GPURasterEffectCreator::sGetBestCompatibleEffects(id);
    if(best.isEmpty()) RuntimeThrow("No compatible GPU effect found for " + id.fName);
    qsptr<GPURasterEffect> effect;
    if(best.count() == 1) {
        const auto bestCreator = best.first();
        effect = GetAsSPtr(bestCreator->create(), GPURasterEffect);
    } else {
        // exec ask dialog
    }
    return effect;
}

bool GPUEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}
