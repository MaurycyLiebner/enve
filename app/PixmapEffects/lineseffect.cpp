#include "lineseffect.h"
#include "Animators/qrealanimator.h"

LinesEffect::LinesEffect(qreal linesWidth, qreal linesDistance) :
    PixmapEffect("lines", EFFECT_LINES) {
    mLinesDistance = SPtrCreate(QrealAnimator)("distance");
    mLinesDistance->setValueRange(0., 100000.);
    mLinesDistance->setCurrentBaseValue(linesDistance);

    mLinesWidth = SPtrCreate(QrealAnimator)("width");
    mLinesWidth->setValueRange(0., 100000.);
    mLinesWidth->setCurrentBaseValue(linesWidth);

    ca_addChildAnimator(mLinesWidth);
    ca_addChildAnimator(mLinesDistance);
}


stdsptr<PixmapEffectRenderData> LinesEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(LinesEffectRenderData)();
    renderData->linesDistance = mLinesDistance->getEffectiveValueAtRelFrame(relFrame);
    renderData->linesWidth = mLinesWidth->getEffectiveValueAtRelFrame(relFrame);
    renderData->vertical = mVertical;

    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void LinesEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                           const qreal &scale) {
    Q_UNUSED(bitmap)
    Q_UNUSED(scale)
}
