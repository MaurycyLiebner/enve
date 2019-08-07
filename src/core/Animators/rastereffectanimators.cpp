#include "rastereffectanimators.h"
#include "RasterEffects/rastereffect.h"
#include "Boxes/boundingbox.h"
#include <QDebug>

RasterEffectAnimators::RasterEffectAnimators(BoundingBox * const parentBox) :
    RasterEffectAnimatorsBase("raster effects"), mParentBox_k(parentBox) {
    makeHiddenWhenEmpty();

    connect(this, &ComplexAnimator::childAdded,
            this, &RasterEffectAnimators::updateUnbound);
    connect(this, &ComplexAnimator::childRemoved,
            this, &RasterEffectAnimators::updateUnbound);
}

QMargins RasterEffectAnimators::getEffectsMargin(const qreal relFrame) const {
    QMargins newMargin;
    for(const auto& effect : ca_mChildAnimators) {
        auto rasterEffect = static_cast<RasterEffect*>(effect.get());
        if(rasterEffect->isVisible()) {
            newMargin += rasterEffect->getMarginAtRelFrame(relFrame);
        }
    }
    return newMargin;
}

void RasterEffectAnimators::updateUnbound() {
    for(const auto& effect : ca_mChildAnimators) {
        auto rasterEffect = static_cast<RasterEffect*>(effect.get());
        if(/*rasterEffect->isVisible() && */rasterEffect->forceMargin()) {
            mUnbound = true;
            return;
        }
    }
    mUnbound = false;
}

bool RasterEffectAnimators::unbound() const {
    return mUnbound;
}

void RasterEffectAnimators::addEffects(const qreal relFrame,
                                       BoxRenderData * const data) {
    for(const auto& effect : ca_mChildAnimators) {
        auto rasterEffect = static_cast<ShaderEffect*>(effect.get());
        if(rasterEffect->isVisible()) {
            const auto effectRenderData =
                    rasterEffect->getEffectCaller(relFrame);
            if(!effectRenderData) continue;
            data->addEffect(effectRenderData);
        }
    }
}

void RasterEffectAnimators::updateIfUsesProgram(
        const ShaderEffectProgram * const program) {
    for(const auto& effect : ca_mChildAnimators) {
        const auto shaderEffect = dynamic_cast<ShaderEffect*>(effect.get());
        if(!shaderEffect) continue;
        shaderEffect->updateIfUsesProgram(program);
    }
}

bool RasterEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}

qsptr<ShaderEffect> readIdCreateShaderEffect(QIODevice * const src) {
    const auto id = ShaderEffectCreator::sReadIdentifier(src);
    const auto best = ShaderEffectCreator::sGetBestCompatibleEffects(id);
    if(best.isEmpty()) RuntimeThrow("No compatible ShaderEffect found for " + id.fName);
    qsptr<ShaderEffect> effect;
    if(best.count() == 1) {
        const auto bestCreator = best.first();
        effect = qSharedPointerCast<ShaderEffect>(bestCreator->create());
    } else {
        // exec ask dialog
    }
    return effect;
}

#include "customidentifier.h"
#include "RasterEffects/customrastereffectcreator.h"
qsptr<RasterEffect> readIdCreateRasterEffect(QIODevice * const src) {
    RasterEffectType type;
    src->read(rcChar(&type), sizeof(RasterEffectType));
    switch(type) {
        case(RasterEffectType::BLUR):
            return nullptr;
        case(RasterEffectType::SHADOW):
            return nullptr;
        case(RasterEffectType::CUSTOM): {
            const auto id = CustomIdentifier::sRead(src);
            return CustomRasterEffectCreator::sCreateForIdentifier(id);
        } case(RasterEffectType::CUSTOM_SHADER):
            return readIdCreateShaderEffect(src);
        default: RuntimeThrow("Invalid RasterEffect type '" +
                              QString::number(int(type)) + "'");
    }
}
