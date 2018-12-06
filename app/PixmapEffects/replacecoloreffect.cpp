#include "replacecoloreffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"
#include "Animators/coloranimator.h"

ReplaceColorEffect::ReplaceColorEffect() :
    PixmapEffect("replace color", EFFECT_REPLACE_COLOR) {
    mFromColor = SPtrCreate(ColorAnimator)("from color");
    mToColor = SPtrCreate(ColorAnimator)("to color");

    mToleranceAnimator = SPtrCreate(QrealAnimator)("tolerance");
    mToleranceAnimator->qra_setValueRange(0., 1.);
    mToleranceAnimator->setPrefferedValueStep(0.01);

    mSmoothnessAnimator = SPtrCreate(QrealAnimator)("smoothness");
    mSmoothnessAnimator->qra_setValueRange(0., 1.);
    mSmoothnessAnimator->setPrefferedValueStep(0.01);

    ca_addChildAnimator(mFromColor);
    ca_addChildAnimator(mToColor);
    ca_addChildAnimator(mToleranceAnimator);
    ca_addChildAnimator(mSmoothnessAnimator);
}

stdsptr<PixmapEffectRenderData> ReplaceColorEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(ReplaceColorEffectRenderData)();
    QColor fromColor = mFromColor->getColorAtRelFrameF(relFrame);
    QColor toColor = mToColor->getColorAtRelFrameF(relFrame);

    renderData->redR = qRound(fromColor.red()*fromColor.alphaF());
    renderData->greenR = qRound(fromColor.green()*fromColor.alphaF());
    renderData->blueR = qRound(fromColor.blue()*fromColor.alphaF());
    renderData->alphaR = fromColor.alpha();

    renderData->redT = qRound(toColor.red()*toColor.alphaF());
    renderData->greenT = qRound(toColor.green()*toColor.alphaF());
    renderData->blueT = qRound(toColor.blue()*toColor.alphaF());
    renderData->alphaT = toColor.alpha();

    renderData->tolerance = qRound(mToleranceAnimator->
            getCurrentEffectiveValueAtRelFrameF(relFrame)*255);
    renderData->smoothness = mSmoothnessAnimator->
            getCurrentEffectiveValueAtRelFrameF(relFrame);
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void ReplaceColorEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                                  const qreal &scale) {
    Q_UNUSED(imgPtr);
    RasterEffects::replaceColor(img, redR, greenR, blueR, alphaR,
                              redT, greenT, blueT, alphaT,
                              tolerance, smoothness*scale);
}
