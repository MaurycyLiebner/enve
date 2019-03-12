#include "paintsettingsapplier.h"
#include "Animators/coloranimator.h"
#include "Boxes/pathbox.h"

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

void PaintSettingsApplier::apply(PathBox * const box) const {
    PaintSettings* paintSettings = nullptr;
    if(mTargetFillSettings) paintSettings = box->getFillSettings();
    else paintSettings = box->getStrokeSettings();
    const bool paintTypeChanged = paintSettings->getPaintType() != mPaintType;
    bool gradientChanged = false;

    if(mPaintType == FLATPAINT || mPaintType == BRUSHPAINT) {
        mColorSetting.apply(paintSettings->getColorAnimator());
    } else if(mPaintType == GRADIENTPAINT) {
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

void PaintSettingsApplier::applyColorSetting(ColorAnimator *animator) const {
    mColorSetting.apply(animator);
}
