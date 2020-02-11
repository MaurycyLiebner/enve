#include "blendeffectcollection.h"

#include "Boxes/layerboxrenderdata.h"
#include "typemenu.h"

#include "moveblendeffect.h"
#include "targetedblendeffect.h"

BlendEffectCollection::BlendEffectCollection() :
    BlendEffectCollectionBase("blend effects") {

}

void BlendEffectCollection::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<BlendEffectCollection>()) return;
    menu->addedActionsForType<BlendEffectCollection>();
    {
        const PropertyMenu::PlainSelectedOp<BlendEffectCollection> aOp =
        [](BlendEffectCollection * coll) {
            coll->addChild(enve::make_shared<MoveBlendEffect>());
        };
        menu->addPlainAction("New Move Effect", aOp);
    }
    {
        const PropertyMenu::PlainSelectedOp<BlendEffectCollection> aOp =
        [](BlendEffectCollection * coll) {
            coll->addChild(enve::make_shared<TargetedBlendEffect>());
        };
        menu->addPlainAction("New Targeted Effect", aOp);
    }
    menu->addSeparator();
    BlendEffectCollectionBase::prp_setupTreeViewMenu(menu);
}

void BlendEffectCollection::blendSetup(
        ChildRenderData &data,
        const int index, const qreal relFrame,
        QList<ChildRenderData> &delayed) const {
    const int iMax = ca_getNumberOfChildren();
    for(int i = 0; i < iMax; i++) {
        const auto effect = getChild(i);
        effect->blendSetup(data, index, relFrame, delayed);
    }
}

void BlendEffectCollection::drawBlendSetup(SkCanvas * const canvas) {
    const qreal relFrame = anim_getCurrentRelFrame();
    const int iMax = ca_getNumberOfChildren();
    for(int i = 0; i < iMax; i++) {
        const auto effect = getChild(i);
        effect->drawBlendSetup(relFrame, canvas);
    }
}

void BlendEffectCollection::detachedBlendSetup(
        BoundingBox* const boxToDraw,
        SkCanvas * const canvas,
        const SkFilterQuality filter, int &drawId,
        QList<BlendEffect::Delayed> &delayed) const {
    const qreal relFrame = anim_getCurrentRelFrame();
    const int iMax = ca_getNumberOfChildren();
    for(int i = 0; i < iMax; i++) {
        const auto effect = getChild(i);
        effect->detachedBlendSetup(boxToDraw, relFrame, canvas, filter, drawId, delayed);
    }
}

void BlendEffectCollection::prp_writeProperty(eWriteStream &dst) const {
    BlendEffectCollectionBase::prp_writeProperty(dst);
    dst << SWT_isVisible();
}

void BlendEffectCollection::prp_readProperty(eReadStream &src) {
    if(src.evFileVersion() < 12) return;
    BlendEffectCollectionBase::prp_readProperty(src);
    bool visible; src >> visible;
    SWT_setVisible(visible);
}

qsptr<BlendEffect> readIdCreateBlendEffect(eReadStream &src) {
    BlendEffectType type;
    src.read(&type, sizeof(BlendEffectType));
    switch(type) {
        case(BlendEffectType::move):
            return enve::make_shared<MoveBlendEffect>();
        case(BlendEffectType::targeted):
            return enve::make_shared<TargetedBlendEffect>();
        default: RuntimeThrow("Invalid blend effect type '" +
                              QString::number(int(type)) + "'");
    }
}

void writeBlendEffectType(BlendEffect * const obj, eWriteStream &dst) {
    obj->writeIdentifier(dst);
}
