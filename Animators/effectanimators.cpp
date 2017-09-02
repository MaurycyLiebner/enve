#include "effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Boxes/boundingbox.h"
#include <QDebug>

EffectAnimators::EffectAnimators() :
    ComplexAnimator() {

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
        ((PixmapEffect*)effect.data())->applySk(imgPtr,
                                                img, scale);
    }
}

qreal EffectAnimators::getEffectsMargin() const {
    qreal newMargin = 2.;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        qreal effectMargin = ((PixmapEffect*)effect.data())->getMargin();
        newMargin += effectMargin;
    }
    return newMargin;
}

qreal EffectAnimators::getEffectsMarginAtRelFrame(const int &relFrame) const {
    qreal newMargin = 0.;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        qreal effectMargin = ((PixmapEffect*)effect.data())->
                getMarginAtRelFrame(relFrame);
        newMargin += effectMargin;
    }
    return newMargin;
}

void EffectAnimators::addEffectRenderDataToList(
        const int &relFrame,
        QList<PixmapEffectRenderData *> *pixmapEffects) {
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        pixmapEffects->append(
                    ((PixmapEffect*)effect.data())->
                     getPixmapEffectRenderDataForRelFrame(relFrame) );
    }
}

bool EffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}

void EffectAnimators::makeDuplicate(Property *target) {
    EffectAnimators *eaTarget = ((EffectAnimators*)target);
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        eaTarget->addEffect((PixmapEffect*)
                    ((PixmapEffect*)effect.data())->makeDuplicate());
    }
}
