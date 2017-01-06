#ifndef EFFECTANIMATORS_H
#define EFFECTANIMATORS_H
#include "complexanimator.h"

namespace fmt_filters { struct image; }

class EffectsSettingsWidget;

class EffectAnimators : public ComplexAnimator
{
public:
    EffectAnimators();

    void applyEffects(QImage *imgPtr,
                      const fmt_filters::image &img,
                      const qreal &scale,
                      const bool &highQuality);
    qreal getEffectsMargin() const;
    void addAllEffectsToEffectsSettingsWidget(EffectsSettingsWidget *widget);

    bool isEffectsAnimator() { return true; }
};

#endif // EFFECTANIMATORS_H
