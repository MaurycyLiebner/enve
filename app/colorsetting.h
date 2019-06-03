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

class ColorSetting {
public:
    ColorSetting();
    ColorSetting(const ColorMode &settingModeT,
                 const CVR_TYPE &changedValueT,
                 const qreal &val1T,
                 const qreal &val2T,
                 const qreal &val3T,
                 const qreal &alphaT,
                 const ColorSettingType &typeT,
                 ColorAnimator * const excludeT = nullptr);
    void apply(ColorAnimator * const target) const;

    const ColorSettingType &getType() const;
    const ColorMode &getSettingMode() const;
    const CVR_TYPE &getChangedValue() const;
    const qreal &getVal1() const;
    const qreal &getVal2() const;
    const qreal &getVal3() const;
    const qreal &getAlpa() const;

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
    void finishColorTransform(ColorAnimator *target) const;

    void changeColor(ColorAnimator *target) const;

    void startColorTransform(ColorAnimator *target) const;
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
