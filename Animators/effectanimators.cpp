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

void EffectAnimators::applyEffectsSk(const SkBitmap &imgPtr,
                                     const fmt_filters::image &img,
                                     const qreal &scale) {
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PixmapEffect *pixmapEffect = ((PixmapEffect*)effect.data());
        if(pixmapEffect->isVisible()) {
            pixmapEffect->applySk(imgPtr, img, scale);
        }
    }
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

void EffectAnimators::addEffectRenderDataToList(
        const int &relFrame,
        BoundingBoxRenderData *data) {
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PixmapEffect *pixmapEffect = ((PixmapEffect*)effect.data());
        if(pixmapEffect->isVisible()) {
            PixmapEffectRenderData *effectRenderData =
                    pixmapEffect->getPixmapEffectRenderDataForRelFrame(relFrame,
                                                                       data);
            if(effectRenderData == NULL) continue;
            data->pixmapEffects.append(effectRenderData);
        }
    }
}

void EffectAnimators::addEffectRenderDataToListF(
        const qreal &relFrame,
        BoundingBoxRenderData *data) {
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PixmapEffect *pixmapEffect = ((PixmapEffect*)effect.data());
        if(pixmapEffect->isVisible()) {
            PixmapEffectRenderData *effectRenderData =
                    pixmapEffect->getPixmapEffectRenderDataForRelFrameF(relFrame,
                                                                        data);
            if(effectRenderData == NULL) continue;
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
