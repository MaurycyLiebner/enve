#include "blendeffect.h"
#include "Boxes/pathbox.h"

BlendEffect::BlendEffect(const QString& name,
                         const BlendEffectType type) :
    eEffect(name), mType(type) {
    mClipPath = enve::make_shared<BoxTargetProperty>("clip path");

    connect(mClipPath.get(), &BoxTargetProperty::targetSet,
            this, [this](BoundingBox* const newClipBox) {
        auto& conn = mClipBox.assign(newClipBox);
        if(newClipBox) {
            conn << connect(newClipBox, &Property::prp_absFrameRangeChanged,
                            this, &Property::prp_afterChangedAbsRange);
        }
    });

    ca_addChild(mClipPath);
}

void BlendEffect::prp_readProperty(eReadStream& src) {
    if(src.evFileVersion() < 13) {
        StaticComplexAnimator::prp_readProperty(src);
    } else eEffect::prp_readProperty(src);
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
