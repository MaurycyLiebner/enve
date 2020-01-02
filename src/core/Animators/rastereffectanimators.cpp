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

#include "rastereffectanimators.h"
#include "RasterEffects/rastereffect.h"
#include "Boxes/boundingbox.h"
#include "Boxes/containerbox.h"
#include "RasterEffects/rastereffectsinclude.h"
#include "RasterEffects/customrastereffectcreator.h"

RasterEffectAnimators::RasterEffectAnimators() :
    RasterEffectAnimatorsBase("raster effects") {
    ca_setHiddenWhenEmpty(true);

    connect(this, &ComplexAnimator::ca_childAdded,
            this, &RasterEffectAnimators::updateMaxForcedMargin);
    connect(this, &ComplexAnimator::ca_childRemoved,
            this, &RasterEffectAnimators::updateMaxForcedMargin);
}

template <typename T>
void addRasterEffectActionToMenu(const QString& text,
                                 PropertyMenu * const menu) {
    menu->addPlainAction<RasterEffectAnimators>(
                text, [](RasterEffectAnimators * target) {
        target->addChild(enve::make_shared<T>());
    });
}

void RasterEffectAnimators::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<RasterEffectAnimators>()) return;
    menu->addedActionsForType<RasterEffectAnimators>();
    const auto rasterEffectsMenu = menu->addMenu("Add Effect");
    addRasterEffectActionToMenu<BlurEffect>("Blur", rasterEffectsMenu);
    addRasterEffectActionToMenu<ShadowEffect>("Shadow", rasterEffectsMenu);
    CustomRasterEffectCreator::sAddToMenu(rasterEffectsMenu,
                                          &BoundingBox::addRasterEffect);
    if(!rasterEffectsMenu->isEmpty()) rasterEffectsMenu->addSeparator();
    for(const auto& creator : ShaderEffectCreator::sEffectCreators) {
        const PropertyMenu::PlainSelectedOp<RasterEffectAnimators> op =
        [creator](RasterEffectAnimators * target) {
            const auto effect = creator->create();
            target->addChild(qSharedPointerCast<RasterEffect>(effect));
        };
        rasterEffectsMenu->addPlainAction(creator->fName, op);
    }
    menu->addSeparator();
    RasterEffectAnimatorsBase::prp_setupTreeViewMenu(menu);
}

void RasterEffectAnimators::updateMaxForcedMargin() {
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

void RasterEffectAnimators::addEffects(const qreal relFrame,
                                       BoxRenderData * const data,
                                       const qreal influence) {
    const auto& children = ca_getChildren();
    for(const auto& effect : children) {
        auto rasterEffect = static_cast<RasterEffect*>(effect.get());
        if(rasterEffect->isVisible()) {
            const auto effectRenderData = rasterEffect->getEffectCaller(
                        relFrame, data->fResolution, influence);
            if(!effectRenderData) continue;
            data->addEffect(effectRenderData);
        }
    }
}

void RasterEffectAnimators::updateIfUsesProgram(
        const ShaderEffectProgram * const program) {
    const auto& children = ca_getChildren();
    for(const auto& effect : children) {
        const auto shaderEffect = dynamic_cast<ShaderEffect*>(effect.get());
        if(!shaderEffect) continue;
        shaderEffect->updateIfUsesProgram(program);
    }
}

bool RasterEffectAnimators::hasEffects() {
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
