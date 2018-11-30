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


PixmapEffectRenderDataSPtr LinesEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(LinesEffectRenderData)();
    renderData->linesDistance = mLinesDistance->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->linesWidth = mLinesWidth->getCurrentEffectiveValueAtRelFrameF(relFrame);
    renderData->vertical = mVertical;

    return renderData;
}

void LinesEffectRenderData::applyEffectsSk(const SkBitmap &imgPtr,
                                           const fmt_filters::image &img,
                                           const qreal &scale) {
    Q_UNUSED(imgPtr)
    Q_UNUSED(img)
    Q_UNUSED(scale)
}
