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

#include "rastereffectcollection.h"
#include "RasterEffects/rastereffect.h"
#include "Boxes/boundingbox.h"
#include "Boxes/containerbox.h"
#include "RasterEffects/rastereffectsinclude.h"
#include "RasterEffects/customrastereffectcreator.h"
#include "rastereffectmenucreator.h"

RasterEffectCollection::RasterEffectCollection() :
    RasterEffectCollectionBase("raster effects") {
    ca_setHiddenWhenEmpty(true);

    connect(this, &ComplexAnimator::ca_childAdded,
            this, &RasterEffectCollection::updateMaxForcedMargin);
    connect(this, &ComplexAnimator::ca_childRemoved,
            this, &RasterEffectCollection::updateMaxForcedMargin);
}

void RasterEffectCollection::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<RasterEffectCollection>()) return;
    menu->addedActionsForType<RasterEffectCollection>();
    const auto rasterEffectsMenu = menu->addMenu("Add Effect");
    RasterEffectMenuCreator::addEffects(
                rasterEffectsMenu, &RasterEffectCollection::addChild);
    menu->addSeparator();
    RasterEffectCollectionBase::prp_setupTreeViewMenu(menu);
}

void RasterEffectCollection::updateMaxForcedMargin() {
    QMargins newMargins;
    const auto& children = ca_getChildren();
    for(const auto& effect : children) {
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

    if(changed) forcedMarginChanged();
}

void RasterEffectCollection::addEffects(const qreal relFrame,
                                       BoxRenderData * const data,
                                       const qreal influence) {
    const bool zeroInfluence = isZero4Dec(influence);
    const auto& children = ca_getChildren();
    for(const auto& effect : children) {
        const auto rEffect = static_cast<RasterEffect*>(effect.get());
        if(!rEffect->isVisible()) continue;
        if(zeroInfluence && rEffect->skipZeroInfluence(relFrame)) continue;
        const auto effectRenderData = rEffect->getEffectCaller(
                    relFrame, data->fResolution, influence);
        if(!effectRenderData) continue;
        data->addEffect(effectRenderData);
    }
}

void RasterEffectCollection::updateIfUsesProgram(
        const ShaderEffectProgram * const program) {
    const auto& children = ca_getChildren();
    for(const auto& effect : children) {
        const auto shaderEffect = enve_cast<ShaderEffect*>(effect.get());
        if(!shaderEffect) continue;
        shaderEffect->updateIfUsesProgram(program);
    }
}

bool RasterEffectCollection::hasEffects() {
    return ca_hasChildren();
}

qsptr<ShaderEffect> readIdCreateShaderEffect(eReadStream& src) {
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
#include "RasterEffects/blureffect.h"
#include "RasterEffects/shadoweffect.h"

qsptr<RasterEffect> readIdCreateRasterEffect(eReadStream &src) {
    RasterEffectType type;
    src.read(&type, sizeof(RasterEffectType));
    switch(type) {
        case(RasterEffectType::BLUR):
            return enve::make_shared<BlurEffect>();
        case(RasterEffectType::SHADOW):
            return enve::make_shared<ShadowEffect>();
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

void writeRasterEffectType(RasterEffect * const obj, eWriteStream &dst) {
    obj->writeIdentifier(dst);
}
