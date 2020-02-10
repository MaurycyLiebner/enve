#include "blendeffect.h"
#include "Boxes/pathbox.h"

BlendEffect::BlendEffect(const BlendEffectType type) :
    StaticComplexAnimator("blend effect"), mType(type) {
    mClipPath = enve::make_shared<BoxTargetProperty>("clip path");

    ca_addChild(mClipPath);
}

void BlendEffect::writeIdentifier(eWriteStream& dst) const {
    dst.write(&mType, sizeof(BlendEffectType));
}

void BlendEffect::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<BlendEffect>()) return;
    menu->addedActionsForType<BlendEffect>();
    {
        const PropertyMenu::PlainSelectedOp<BlendEffect> dOp =
        [](BlendEffect* const prop) {
            const auto parent = prop->template getParent<
                    DynamicComplexAnimatorBase<BlendEffect>>();
            parent->removeChild(prop->template ref<BlendEffect>());
        };
        menu->addPlainAction("Delete", dOp);
    }

    menu->addSeparator();
    StaticComplexAnimator::prp_setupTreeViewMenu(menu);
}

bool BlendEffect::isPathValid() const {
    return clipPathSource();
}

SkPath BlendEffect::clipPath(const qreal relFrame) const {
    const auto target = clipPathSource();
    if(!target) return SkPath();
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal tRelFrame = target->prp_absFrameToRelFrameF(absFrame);
    return target->getAbsolutePath(tRelFrame);
}

PathBox *BlendEffect::clipPathSource() const {
    const auto target = mClipPath->getTarget();
    if(!target) return nullptr;
    if(!target->SWT_isPathBox()) return nullptr;
    return static_cast<PathBox*>(target);
}
