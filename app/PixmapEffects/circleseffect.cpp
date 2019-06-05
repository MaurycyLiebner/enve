#include "circleseffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"

CirclesEffect::CirclesEffect(qreal circlesRadius,
                             qreal circlesDistance) :
    PixmapEffect("circles", EFFECT_CIRCLES) {
    mCirclesDistance = SPtrCreate(QrealAnimator)("distance");
    mCirclesDistance->setValueRange(-1000., 1000.);
    mCirclesDistance->setCurrentBaseValue(circlesDistance);

    mCirclesRadius = SPtrCreate(QrealAnimator)("radius");
    mCirclesRadius->setValueRange(0., 1000.);
    mCirclesRadius->setCurrentBaseValue(circlesRadius);

    ca_addChildAnimator(mCirclesRadius);
    ca_addChildAnimator(mCirclesDistance);
}

stdsptr<PixmapEffectRenderData> CirclesEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(CirclesEffectRenderData)();
    renderData->circlesDistance =
            mCirclesDistance->getEffectiveValue(relFrame);
    renderData->circlesRadius =
            mCirclesRadius->getEffectiveValue(relFrame);

    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void CirclesEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                             const qreal scale) {
    Q_UNUSED(bitmap)
    Q_UNUSED(scale)
}
