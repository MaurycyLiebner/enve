#include "rastereffectanimators.h"
#include "RasterEffects/rastereffect.h"
#include "Boxes/boundingbox.h"
#include "Boxes/containerbox.h"

RasterEffectAnimators::RasterEffectAnimators(BoundingBox * const parentBox) :
    RasterEffectAnimatorsBase("raster effects"), mParentBox_k(parentBox) {
    makeHiddenWhenEmpty();

    connect(this, &ComplexAnimator::childAdded,
            this, &RasterEffectAnimators::updateMaxForcedMargin);
    connect(this, &ComplexAnimator::childRemoved,
            this, &RasterEffectAnimators::updateMaxForcedMargin);
}

void RasterEffectAnimators::updateMaxForcedMargin() {
    QMargins newMargins;
    for(const auto& effect : ca_mChildAnimators) {
        const auto rasterEffect = static_cast<RasterEffect*>(effect.get());
        if(!rasterEffect->forceMargin()) continue;
        const auto reMargin = rasterEffect->getMargin();
        newMargins.setTop(qMax(newMargins.top(), reMargin.top()));
        newMargins.setLeft(qMax(newMargins.left(), reMargin.left()));
        newMargins.setBottom(qMax(newMargins.bottom(), reMargin.bottom()));
        newMargins.setRight(qMax(newMargins.right(), reMargin.right()));
    }
    newMargins.setTop(qCeil(newMargins.top()*0.1)*10);
    newMargins.setLeft(qCeil(newMargins.left()*0.1)*10);
    newMargins.setBottom(qCeil(newMargins.bottom()*0.1)*10);
    newMargins.setRight(qCeil(newMargins.right()*0.1)*10);

    const bool changed =
            newMargins.top() > mMaxForcedMargin.top() ||
            newMargins.left() > mMaxForcedMargin.left() ||
            newMargins.bottom() > mMaxForcedMargin.bottom() ||
            newMargins.right() > mMaxForcedMargin.right();

    mMaxForcedMargin = newMargins;

    if(changed && mParentBox_k->SWT_isLayerBox()) {
        const auto cont = static_cast<ContainerBox*>(mParentBox_k);
        cont->forcedMarginMeaningfulChange();
    }
}

void RasterEffectAnimators::addEffects(const qreal relFrame,
                                       BoxRenderData * const data) {
    for(const auto& effect : ca_mChildAnimators) {
        auto rasterEffect = static_cast<RasterEffect*>(effect.get());
        if(rasterEffect->isVisible()) {
            const auto effectRenderData = rasterEffect->getEffectCaller(relFrame);
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
        effect = bestCreator->create();
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
            const auto eff = CustomRasterEffectCreator::sCreateForIdentifier(id);
            if(eff) return eff;
            RuntimeThrow("Unrecogized CustomRasterEffect identifier " + id.toString());
        } case(RasterEffectType::CUSTOM_SHADER):
            return readIdCreateShaderEffect(src);
        default: RuntimeThrow("Invalid RasterEffect type '" +
                              QString::number(int(type)) + "'");
    }
}
