#include "blendeffectboxshadow.h"

#include "Boxes/boundingbox.h"

BlendEffectBoxShadow::BlendEffectBoxShadow(BoundingBox* const box,
                                           BlendEffect * const effect) :
    Property(box->prp_getName()), mBox(box), mEffect(effect) {
    connect(box, &Property::prp_nameChanged,
            this, &Property::prp_setName);
}


bool BlendEffectBoxShadow::SWT_shouldBeVisible(
        const SWT_RulesCollection &rules,
        const bool parentSatisfies,
        const bool parentMainTarget) const {
    return mBox->SWT_shouldBeVisible(
                rules, parentSatisfies, parentMainTarget);
}
