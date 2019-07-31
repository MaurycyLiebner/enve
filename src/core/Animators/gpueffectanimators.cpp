#include "gpueffectanimators.h"
#include "GPUEffects/gpueffect.h"
#include "Boxes/boundingbox.h"
#include <QDebug>

GPUEffectAnimators::GPUEffectAnimators(BoundingBox * const parentBox) :
    GPUEffectAnimatorsBase("gpu effects"), mParentBox_k(parentBox) {
    makeHiddenWhenEmpty();

    connect(this, &ComplexAnimator::childAdded,
            this, &GPUEffectAnimators::updateUnbound);
    connect(this, &ComplexAnimator::childRemoved,
            this, &GPUEffectAnimators::updateUnbound);
}

QMargins GPUEffectAnimators::getEffectsMargin(const qreal relFrame) const {
    QMargins newMargin;
    for(const auto& effect : ca_mChildAnimators) {
        auto rasterEffect = GetAsPtr(effect.get(), GpuEffect);
        if(rasterEffect->isVisible()) {
            newMargin += rasterEffect->getMarginAtRelFrame(relFrame);
        }
    }
    return newMargin;
}

void GPUEffectAnimators::updateUnbound() {
    for(const auto& effect : ca_mChildAnimators) {
        auto gpuEffect = GetAsPtr(effect.get(), GpuEffect);
        if(gpuEffect->isVisible() && gpuEffect->forceMargin()) {
            mUnbound = true;
            return;
        }
    }
    mUnbound = false;
}

bool GPUEffectAnimators::unbound() const {
    return mUnbound;
}

void GPUEffectAnimators::addEffects(const qreal relFrame,
                                    BoxRenderData * const data) {
    for(const auto& effect : ca_mChildAnimators) {
        auto rasterEffect = GetAsPtr(effect, ShaderEffect);
        if(rasterEffect->isVisible()) {
            const auto effectRenderData =
                    rasterEffect->getEffectCaller(relFrame);
            if(!effectRenderData) continue;
            data->addEffect(effectRenderData);
        }
    }
}

void GPUEffectAnimators::updateIfUsesProgram(
        const ShaderEffectProgram * const program) {
    for(const auto& effect : ca_mChildAnimators) {
        if(!effect->SWT_isShaderEffect()) continue;
        const auto rasterEffect = GetAsPtr(effect.get(), ShaderEffect);
        rasterEffect->updateIfUsesProgram(program);
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

#include "customidentifier.h"
qsptr<GpuEffect> readIdCreateGPURasterEffect(QIODevice * const src) {
    GpuEffectType type;
    src->read(rcChar(&type), sizeof(GpuEffectType));
    switch(type) {
        case(GpuEffectType::BLUR):
            return nullptr;
        case(GpuEffectType::SHADOW):
            return nullptr;
        case(GpuEffectType::CUSTOM): {
            const auto id = CustomIdentifier::sRead(src);
            return nullptr;
            //return CustomGpuEffectCreator::sCreateForIdentifier(id);
        } case(GpuEffectType::CUSTOM_SHADER):
            return readIdCreateShaderEffect(src);
        default: RuntimeThrow("Invalid gpu effect type '" +
                              QString::number(int(type)) + "'");
    }
}

bool GPUEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}
