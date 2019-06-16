#ifndef COLORSETTING_H
#define COLORSETTING_H
class ColorAnimator;
#include "colorhelpers.h"
#include "smartPointers/sharedpointerdefs.h"
enum ColorSettingType : short {
    CST_START,
    CST_CHANGE,
    CST_FINISH
};

enum CVR_TYPE : short;

class ColorSettingApplier {
public:
    ColorSettingApplier();
    ColorSettingApplier(const ColorMode settingModeT,
                 const CVR_TYPE changedValueT,
                 const qreal val1T,
                 const qreal val2T,
                 const qreal val3T,
                 const qreal alphaT,
                 const ColorSettingType typeT,
                 ColorAnimator * const excludeT = nullptr);
    void apply(ColorAnimator * const target) const;

    QColor getColor() const {
        QColor color;
        if(mSettingMode == RGBMODE) {
            color.setRgbF(mVal1, mVal2, mVal3, mAlpha);
        } else if(mSettingMode == HSVMODE) {
            color.setHsvF(mVal1, mVal2, mVal3, mAlpha);
        } else if(mSettingMode == HSLMODE) {
            color.setHslF(mVal1, mVal2, mVal3, mAlpha);
        }
        return color;
    }
private:
    void startColorTransform(ColorAnimator* const target) const;
    void finishColorTransform(ColorAnimator* const target) const;
    void changeColor(ColorAnimator* const target) const;

    ColorSettingType mType = CST_FINISH;
    ColorMode mSettingMode = RGBMODE;
    CVR_TYPE mChangedValue;
    qreal mVal1 = 1;
    qreal mVal2 = 1;
    qreal mVal3 = 1;
    qreal mAlpha = 1;
    qptr<ColorAnimator> mExclude;
};
#endif // COLORSETTING_H
