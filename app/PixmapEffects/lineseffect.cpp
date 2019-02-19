#include "lineseffect.h"
#include "Animators/qrealanimator.h"

LinesEffect::LinesEffect(qreal linesWidth, qreal linesDistance) :
    PixmapEffect("lines", EFFECT_LINES) {
    mLinesDistance = SPtrCreate(QrealAnimator)("distance");
    mLinesDistance->qra_setValueRange(0., 100000.);
    mLinesDistance->qra_setCurrentValue(linesDistance);

    mLinesWidth = SPtrCreate(QrealAnimator)("width");
    mLinesWidth->qra_setValueRange(0., 100000.);
    mLinesWidth->qra_setCurrentValue(linesWidth);

    ca_addChildAnimator(mLinesWidth);
    ca_addChildAnimator(mLinesDistance);
}


stdsptr<PixmapEffectRenderData> LinesEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(LinesEffectRenderData)();
    renderData->linesDistance = mLinesDistance->getCurrentEffectiveValueAtRelFrame(relFrame);
    renderData->linesWidth = mLinesWidth->getCurrentEffectiveValueAtRelFrame(relFrame);
    renderData->vertical = mVertical;

    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void LinesEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                           const qreal &scale) {
    Q_UNUSED(bitmap)
    Q_UNUSED(scale)
}
