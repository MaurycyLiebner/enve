#include "effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Boxes/boundingbox.h"
#include <QDebug>

EffectAnimators::EffectAnimators(BoundingBox *parentBox) :
    ComplexAnimator() {
    setParentBox(parentBox);
}

void EffectAnimators::addEffect(PixmapEffect *effect) {
    mParentBox->addEffect(effect);
}

void EffectAnimators::setParentBox(BoundingBox *box) {
    mParentBox = box;
}

qreal EffectAnimators::getEffectsMargin() const {
    qreal newMargin = 2.;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PixmapEffect *pixmapEffect = ((PixmapEffect*)effect.data());
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMargin();
        }
    }
    return newMargin;
}

qreal EffectAnimators::getEffectsMarginAtRelFrame(const int &relFrame) const {
    qreal newMargin = 0.;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PixmapEffect *pixmapEffect = ((PixmapEffect*)effect.data());
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMarginAtRelFrame(relFrame);
        }
    }
    return newMargin;
}

qreal EffectAnimators::getEffectsMarginAtRelFrameF(const qreal &relFrame) const {
    qreal newMargin = 0.;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PixmapEffect *pixmapEffect = ((PixmapEffect*)effect.data());
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMarginAtRelFrame(relFrame);
        }
    }
    return newMargin;
}

void EffectAnimators::addEffectRenderDataToListF(
        const qreal &relFrame,
        const std::shared_ptr<BoundingBoxRenderData>& data) {
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PixmapEffect *pixmapEffect = ((PixmapEffect*)effect.data());
        if(pixmapEffect->isVisible()) {
            PixmapEffectRenderData *effectRenderData =
                    pixmapEffect->getPixmapEffectRenderDataForRelFrameF(relFrame,
                                                                        data);
            if(effectRenderData == nullptr) continue;
            data->pixmapEffects.append(effectRenderData);
        }
    }
}

void EffectAnimators::ca_removeAllChildAnimators() {
    ComplexAnimator::ca_removeAllChildAnimators();
    mParentBox->ca_removeChildAnimator(this);
}

bool EffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}
