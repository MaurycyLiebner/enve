#include "paintsettingsapplier.h"
#include "Animators/coloranimator.h"
#include "Boxes/pathbox.h"
#include "Animators/gradient.h"

PaintSettingsApplier::PaintSettingsApplier(const bool &targetFillSettings,
                           const PaintType &paintType) :
    mTargetFillSettings(targetFillSettings) {
    mPaintType = paintType;
}

PaintSettingsApplier::PaintSettingsApplier(const bool &targetFillSettings,
                           const ColorSetting &colorSetting,
                           const PaintType &paintType) :
    PaintSettingsApplier(targetFillSettings, paintType) {
    mColorSetting = colorSetting;
}

PaintSettingsApplier::PaintSettingsApplier(const bool &targetFillSettings,
                           const ColorSetting &colorSetting) :
    PaintSettingsApplier(targetFillSettings, FLATPAINT) {
    mColorSetting = colorSetting;
}

PaintSettingsApplier::PaintSettingsApplier(const bool &targetFillSettings,
                           const bool &linearGradient,
                           Gradient * const gradient) :
    PaintSettingsApplier(targetFillSettings, GRADIENTPAINT) {
    mLinearGradient = linearGradient;
    mGradient = gradient;
}

void PaintSettingsApplier::apply(PathBox * const box,
                                 const bool& applyAll) const {
    PaintSettings * const paintSettings = mTargetFillSettings ?
                box->getFillSettings() : box->getStrokeSettings();
    const bool paintTypeChanged = paintSettings->getPaintType() != mPaintType;
    bool gradientChanged = false;

    if(mPaintType == FLATPAINT || mPaintType == BRUSHPAINT || applyAll) {
        mColorSetting.apply(paintSettings->getColorAnimator());
    }

    if(mPaintType == GRADIENTPAINT || applyAll) {
        if(paintTypeChanged) gradientChanged = true;
        else {
            gradientChanged = paintSettings->getGradient() == mGradient ||
                    paintSettings->getGradientLinear() == mLinearGradient;
        }
        paintSettings->setGradient(mGradient);
        paintSettings->setGradientLinear(mLinearGradient);
        if(paintTypeChanged) {
            if(mTargetFillSettings) box->resetFillGradientPointsPos();
            else box->resetStrokeGradientPointsPos();
        }
//        if(gradientChanged) {
//            box->updateDrawGradients();
//        }
    }
    if(paintTypeChanged) {
        paintSettings->setPaintType(mPaintType);
        box->clearAllCache();
    }
    if(gradientChanged) {
        if(mTargetFillSettings) box->updateFillDrawGradient();
        else box->updateStrokeDrawGradient();
    }
}

void PaintSettingsApplier::applyColorSetting(
        ColorAnimator * const animator) const {
    mColorSetting.apply(animator);
}

StrokeSettingsApplier::StrokeSettingsApplier(const ColorSetting &colorSetting,
                                             const qreal &width,
                                             SimpleBrushWrapper * const brush,
                                             const qCubicSegment1D& widthCurve,
                                             const qCubicSegment1D& pressureCurve,
                                             const qCubicSegment1D& timeCurve) :
    mPaintSettings(true, colorSetting, PaintType::BRUSHPAINT),
    mWidth(width), mStrokeBrush(brush),
    mWidthCurve(widthCurve), mPressureCurve(pressureCurve),
    mTimeCurve(timeCurve) {

}

void StrokeSettingsApplier::apply(PathBox * const box,
                                  const bool& applyAll) const {
    mPaintSettings.apply(box, applyAll);
    box->setStrokeBrush(mStrokeBrush);
    box->setOutlineCompositionMode(mOutlineCompositionMode);
    box->setStrokeWidth(mWidth);
    box->setStrokeBrushWidthCurve(mWidthCurve);
    box->setStrokeBrushPressureCurve(mPressureCurve);
    box->setStrokeBrushTimeCurve(mTimeCurve);
}
