#include "effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Boxes/boundingbox.h"
#include <QDebug>

EffectAnimators::EffectAnimators(BoundingBox *parentBox) :
    EffectAnimatorsBase("effects"), mParentBox_k(parentBox) {
    makeHiddenWhenEmpty();
}

void EffectAnimators::readPixmapEffect(QIODevice * const src) {
    const auto effect = readIdCreatePixmapEffect(src);
    effect->readProperty(src);
    addChild(effect);
}

qreal EffectAnimators::getEffectsMargin() const {
    qreal newMargin = 2;
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect.get(), PixmapEffect);
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMargin();
        }
    }
    return newMargin;
}

qreal EffectAnimators::getEffectsMarginAtRelFrame(const int relFrame) const {
    qreal newMargin = 0;
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect.get(), PixmapEffect);
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMarginAtRelFrame(relFrame);
        }
    }
    return newMargin;
}

qreal EffectAnimators::getEffectsMarginAtRelFrameF(const qreal relFrame) const {
    qreal newMargin = 0;
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect.get(), PixmapEffect);
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMarginAtRelFrame(qRound(relFrame));
        }
    }
    return newMargin;
}

void EffectAnimators::addEffectRenderDataToListF(
        const qreal relFrame,
        BoundingBoxRenderData * const data) {
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect, PixmapEffect);
        if(pixmapEffect->isVisible()) {
            stdsptr<PixmapEffectRenderData> effectRenderData =
                    pixmapEffect->getPixmapEffectRenderDataForRelFrameF(relFrame, data);
            if(!effectRenderData) continue;
            data->fRasterEffects.append(effectRenderData);
        }
    }
}

bool EffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}

#include "PixmapEffects/blureffect.h"
#include "PixmapEffects/shadoweffect.h"
#include "PixmapEffects/desaturateeffect.h"
#include "PixmapEffects/colorizeeffect.h"
#include "PixmapEffects/replacecoloreffect.h"
#include "PixmapEffects/brightnesseffect.h"
#include "PixmapEffects/contrasteffect.h"
#include "PixmapEffects/sampledmotionblureffect.h"

qsptr<PixmapEffect> readIdCreatePixmapEffect(QIODevice * const src) {
    PixmapEffectType typeT;
    src->read(rcChar(&typeT), sizeof(PixmapEffectType));
    qsptr<PixmapEffect> effect;
    if(typeT == EFFECT_BLUR) {
        effect = SPtrCreate(BlurEffect)();
    } else if(typeT == EFFECT_SHADOW) {
        effect = SPtrCreate(ShadowEffect)();
    } else if(typeT == EFFECT_DESATURATE) {
        effect = SPtrCreate(DesaturateEffect)();
    } else if(typeT == EFFECT_COLORIZE) {
        effect = SPtrCreate(ColorizeEffect)();
    } else if(typeT == EFFECT_REPLACE_COLOR) {
        effect = SPtrCreate(ReplaceColorEffect)();
    } else if(typeT == EFFECT_BRIGHTNESS) {
        effect = SPtrCreate(BrightnessEffect)();
    } else if(typeT == EFFECT_CONTRAST) {
        effect = SPtrCreate(ContrastEffect)();
    } else if(typeT == EFFECT_MOTION_BLUR) {
        effect = SPtrCreate(SampledMotionBlurEffect)(nullptr);
    } else {
        RuntimeThrow("Invalid pixmap effect type '" +
                     QString::number(typeT) + "'.");
    }
    return effect;
}
