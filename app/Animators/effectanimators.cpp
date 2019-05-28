#include "effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Boxes/boundingbox.h"
#include <QDebug>

EffectAnimators::EffectAnimators(BoundingBox *parentBox) :
    ComplexAnimator("effects"), mParentBox_k(parentBox) {
    SWT_setEnabled(false);
    SWT_setVisible(false);
}

void EffectAnimators::addEffect(const qsptr<PixmapEffect>& effect) {
    ca_addChildAnimator(effect);
    effect->setParentEffectAnimators(this);

    prp_afterWholeInfluenceRangeChanged();

    SWT_setEnabled(true);
    SWT_setVisible(true);
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

qreal EffectAnimators::getEffectsMarginAtRelFrame(const int &relFrame) const {
    qreal newMargin = 0;
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect.get(), PixmapEffect);
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMarginAtRelFrame(relFrame);
        }
    }
    return newMargin;
}

qreal EffectAnimators::getEffectsMarginAtRelFrameF(const qreal &relFrame) const {
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
        const qreal &relFrame,
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
