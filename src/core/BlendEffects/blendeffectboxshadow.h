#ifndef BLENDEFFECTBOXSHADOW_H
#define BLENDEFFECTBOXSHADOW_H
#include "Properties/property.h"

class BlendEffect;

class BlendEffectBoxShadow : public Property {
    e_OBJECT
    Q_OBJECT
protected:
    BlendEffectBoxShadow(BoundingBox * const box,
                         BlendEffect* const effect);
public:
    bool SWT_shouldBeVisible(
            const SWT_RulesCollection &rules,
            const bool parentSatisfies,
            const bool parentMainTarget) const;
private:
    BoundingBox* const mBox;
    BlendEffect* const mEffect;
};

#endif // BLENDEFFECTBOXSHADOW_H
