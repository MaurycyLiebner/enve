#include "circleseffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"

CirclesEffect::CirclesEffect(qreal circlesRadius,
                             qreal circlesDistance) :
    PixmapEffect("circles", EFFECT_CIRCLES) {
    mCirclesDistance = SPtrCreate(QrealAnimator)("distance");
    mCirclesDistance->qra_setValueRange(-1000., 1000.);
    mCirclesDistance->qra_setCurrentValue(circlesDistance);

    mCirclesRadius = SPtrCreate(QrealAnimator)("radius");
    mCirclesRadius->qra_setValueRange(0., 1000.);
    mCirclesRadius->qra_setCurrentValue(circlesRadius);

    ca_addChildAnimator(mCirclesRadius);
    ca_addChildAnimator(mCirclesDistance);
}

stdsptr<PixmapEffectRenderData> CirclesEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(CirclesEffectRenderData)();
    renderData->circlesDistance =
            mCirclesDistance->getCurrentEffectiveValueAtRelFrame(relFrame);
    renderData->circlesRadius =
            mCirclesRadius->getCurrentEffectiveValueAtRelFrame(relFrame);

    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void CirclesEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                             const qreal &scale) {
    Q_UNUSED(bitmap)
    Q_UNUSED(scale)
}
