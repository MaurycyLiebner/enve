#include "replacecoloreffect.h"
#include "Animators/qrealanimator.h"
#include "rastereffects.h"
#include "Animators/coloranimator.h"

ReplaceColorEffect::ReplaceColorEffect() :
    PixmapEffect("replace color", EFFECT_REPLACE_COLOR) {
    mFromColor = SPtrCreate(ColorAnimator)("from color");
    mToColor = SPtrCreate(ColorAnimator)("to color");

    mToleranceAnimator = QrealAnimator::create0to1Animator("tolerance");

    mSmoothnessAnimator = QrealAnimator::create0to1Animator("smoothness");

    ca_addChildAnimator(mFromColor);
    ca_addChildAnimator(mToColor);
    ca_addChildAnimator(mToleranceAnimator);
    ca_addChildAnimator(mSmoothnessAnimator);
}

stdsptr<PixmapEffectRenderData> ReplaceColorEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(ReplaceColorEffectRenderData)();
    QColor fromColor = mFromColor->getColorAtRelFrame(relFrame);
    QColor toColor = mToColor->getColorAtRelFrame(relFrame);

    renderData->redR = qRound(fromColor.red()*fromColor.alphaF());
    renderData->greenR = qRound(fromColor.green()*fromColor.alphaF());
    renderData->blueR = qRound(fromColor.blue()*fromColor.alphaF());
    renderData->alphaR = fromColor.alpha();

    renderData->redT = qRound(toColor.red()*toColor.alphaF());
    renderData->greenT = qRound(toColor.green()*toColor.alphaF());
    renderData->blueT = qRound(toColor.blue()*toColor.alphaF());
    renderData->alphaT = toColor.alpha();

    renderData->tolerance = qRound(mToleranceAnimator->
            getEffectiveValue(relFrame)*255);
    renderData->smoothness = mSmoothnessAnimator->
            getEffectiveValue(relFrame);
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void ReplaceColorEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                                  const qreal scale) {
    RasterEffects::replaceColor(bitmap, redR, greenR, blueR, alphaR,
                                redT, greenT, blueT, alphaT,
                                tolerance, smoothness*scale);
}
