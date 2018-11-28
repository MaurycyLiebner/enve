#include "effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Boxes/boundingbox.h"
#include <QDebug>

EffectAnimators::EffectAnimators(BoundingBox *parentBox) :
    ComplexAnimator("effects"), mParentBox_k(parentBox) {}

void EffectAnimators::addEffect(const PixmapEffectQSPtr& effect) {
    mParentBox_k->addEffect(effect);
}

qreal EffectAnimators::getEffectsMargin() const {
    qreal newMargin = 2.;
    Q_FOREACH(const PropertyQSPtr &effect, ca_mChildAnimators) {
        auto pixmapEffect = getAsPtr(effect.get(), PixmapEffect);
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMargin();
        }
    }
    return newMargin;
}

qreal EffectAnimators::getEffectsMarginAtRelFrame(const int &relFrame) const {
    qreal newMargin = 0.;
    Q_FOREACH(const PropertyQSPtr &effect, ca_mChildAnimators) {
        auto pixmapEffect = getAsPtr(effect.get(), PixmapEffect);
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMarginAtRelFrame(relFrame);
        }
    }
    return newMargin;
}

qreal EffectAnimators::getEffectsMarginAtRelFrameF(const qreal &relFrame) const {
    qreal newMargin = 0.;
    Q_FOREACH(const PropertyQSPtr &effect, ca_mChildAnimators) {
        auto pixmapEffect = getAsPtr(effect.get(), PixmapEffect);
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMarginAtRelFrame(qRound(relFrame));
        }
    }
    return newMargin;
}

void EffectAnimators::addEffectRenderDataToListF(
        const qreal &relFrame,
        BoundingBoxRenderData* data) {
    Q_FOREACH(const PropertyQSPtr &effect, ca_mChildAnimators) {
        auto pixmapEffect = getAsPtr(effect, PixmapEffect);
        if(pixmapEffect->isVisible()) {
            PixmapEffectRenderDataSPtr effectRenderData =
                    pixmapEffect->getPixmapEffectRenderDataForRelFrameF(relFrame, data);
            if(effectRenderData == nullptr) continue;
            data->pixmapEffects.append(effectRenderData);
        }
    }
}

void EffectAnimators::ca_removeAllChildAnimators() {
    ComplexAnimator::ca_removeAllChildAnimators();
    mParentBox_k->ca_removeChildAnimator(ref<BoundingBox>());
}

bool EffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}
