#include "effectanimators.h"
#include "pixmapeffect.h"
#include "ObjectSettings/effectssettingswidget.h"

EffectAnimators::EffectAnimators() :
    ComplexAnimator() {

}

void EffectAnimators::applyEffects(QImage *imgPtr,
                                   const fmt_filters::image &img,
                                   const qreal &scale,
                                   const bool &highQuality) {
    foreach(QrealAnimator *effect, mChildAnimators) {
        ((PixmapEffect*)effect)->apply(imgPtr, img, scale, highQuality);
    }
}

qreal EffectAnimators::getEffectsMargin() const {
    qreal newMargin = 5.;
    foreach(QrealAnimator *effect, mChildAnimators) {
        qreal effectMargin = ((PixmapEffect*)effect)->getMargin();
        newMargin += effectMargin;
    }
    return newMargin;
}

void EffectAnimators::addAllEffectsToEffectsSettingsWidget(
        EffectsSettingsWidget *widget) {
    foreach(QrealAnimator *effect, mChildAnimators) {
        widget->addWidgetForEffect((PixmapEffect*)effect);
    }
}
