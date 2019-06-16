#include "colorsetting.h"
#include "Animators/coloranimator.h"

ColorSettingApplier::ColorSettingApplier() {
    mChangedValue = CVR_ALL;
}

ColorSettingApplier::ColorSettingApplier(const ColorMode settingModeT,
                           const CVR_TYPE changedValueT,
                           const qreal val1T,
                           const qreal val2T,
                           const qreal val3T,
                           const qreal alphaT,
                           const ColorSettingType typeT,
                           ColorAnimator * const excludeT) {
    mType = typeT;
    mSettingMode = settingModeT;
    mChangedValue = changedValueT;
    mVal1 = val1T;
    mVal2 = val2T;
    mVal3 = val3T;
    mAlpha = alphaT;
    mExclude = excludeT;
}

void ColorSettingApplier::apply(ColorAnimator * const target) const {
    if(target == mExclude) return;
    if(mType == CST_START) {
        startColorTransform(target);
    } else if(mType == CST_CHANGE) {
        changeColor(target);
    } else {
        finishColorTransform(target);
    }
}

void ColorSettingApplier::finishColorTransform(ColorAnimator* const target) const {
    changeColor(target);
    target->prp_finishTransform();
}

void ColorSettingApplier::changeColor(ColorAnimator* const target) const {
    const ColorMode targetMode = target->getColorMode();
    if(targetMode == mSettingMode) {
        target->setCurrentVal1Value(mVal1);
        target->setCurrentVal2Value(mVal2);
        target->setCurrentVal3Value(mVal3);
    } else {
        qreal val1 = mVal1;
        qreal val2 = mVal2;
        qreal val3 = mVal3;
        if(targetMode == RGBMODE) {
            if(mSettingMode == HSVMODE) {
                qhsv_to_rgb(val1, val2, val3);
            } else {
                qhsl_to_rgb(val1, val2, val3);
            }
        } else if(targetMode == HSVMODE) {
            if(mSettingMode == HSLMODE) {
                qhsl_to_hsv(val1, val2, val3);
            } else {
                qhsl_to_hsv(val1, val2, val3);
            }
        } else if(targetMode == HSLMODE) {
            if(mSettingMode == HSVMODE) {
                qhsv_to_hsl(val1, val2, val3);
            } else {
                qrgb_to_hsl(val1, val2, val3);
            }
        }
        target->setCurrentVal1Value(val1);
        target->setCurrentVal2Value(val2);
        target->setCurrentVal3Value(val3);
    }
    target->setCurrentAlphaValue(mAlpha);
}

void ColorSettingApplier::startColorTransform(ColorAnimator* const target) const {
    const ColorMode &targetMode = target->getColorMode();
    if(targetMode == mSettingMode && mChangedValue != CVR_ALL) {
        if(mChangedValue == CVR_RED || mChangedValue == CVR_HUE) {
            target->startVal1Transform();

            qreal targetVal2 = target->getVal2Animator()->getCurrentBaseValue();
            if(qAbs(targetVal2 - mVal2) > 0.001) {
                target->startVal2Transform();
            }
            qreal targetVal3 = target->getVal3Animator()->getCurrentBaseValue();
            if(qAbs(targetVal3 - mVal3) > 0.001) {
                target->startVal3Transform();
            }
        } else if(mChangedValue == CVR_GREEN ||
                  mChangedValue == CVR_HSVSATURATION ||
                  mChangedValue == CVR_HSLSATURATION) {
            target->startVal2Transform();

            qreal targetVal1 = target->getVal1Animator()->getCurrentBaseValue();
            if(qAbs(targetVal1 - mVal1) > 0.001) {
                target->startVal1Transform();
            }
            qreal targetVal3 = target->getVal3Animator()->getCurrentBaseValue();
            if(qAbs(targetVal3 - mVal3) > 0.001) {
                target->startVal3Transform();
            }
        } else if(mChangedValue == CVR_BLUE ||
                  mChangedValue == CVR_VALUE ||
                  mChangedValue == CVR_LIGHTNESS) {
            target->startVal3Transform();

            qreal targetVal1 = target->getVal1Animator()->getCurrentBaseValue();
            if(qAbs(targetVal1 - mVal1) > 0.001) {
                target->startVal1Transform();
            }
            qreal targetVal2 = target->getVal2Animator()->getCurrentBaseValue();
            if(qAbs(targetVal2 - mVal2) > 0.001) {
                target->startVal2Transform();
            }
        }
    } else {
        target->startVal1Transform();
        target->startVal2Transform();
        target->startVal3Transform();
    }
    qreal targetAlpha = target->getAlphaAnimator()->getCurrentBaseValue();
    if(qAbs(targetAlpha - mAlpha) > 0.001 ||
            mChangedValue == CVR_ALL ||
            mChangedValue == CVR_ALPHA) {
        target->startAlphaTransform();
    }
    changeColor(target);
}
