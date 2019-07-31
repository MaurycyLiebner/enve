#ifndef COLORSETTING_H
#define COLORSETTING_H
class ColorAnimator;
#include <QColor>
#include "colorhelpers.h"
#include "smartPointers/sharedpointerdefs.h"
enum class ColorSettingType : short {
    start, change, finish, apply
};

enum class ColorParameter : short;

class ColorSetting {
public:
    ColorSetting(const ColorMode settingModeT,
                 const ColorParameter changedValueT,
                 const qreal val1T,
                 const qreal val2T,
                 const qreal val3T,
                 const qreal alphaT,
                 const ColorSettingType typeT,
                 ColorAnimator * const excludeT = nullptr);
    void apply(ColorAnimator * const target) const;

    QColor getColor() const {
        QColor color;
        if(fSettingMode == ColorMode::rgb) {
            color.setRgbF(fVal1, fVal2, fVal3, fAlpha);
        } else if(fSettingMode == ColorMode::hsv) {
            color.setHsvF(fVal1, fVal2, fVal3, fAlpha);
        } else if(fSettingMode == ColorMode::hsl) {
            color.setHslF(fVal1, fVal2, fVal3, fAlpha);
        }
        return color;
    }

    ColorSettingType fType;
    ColorMode fSettingMode;
    ColorParameter fChangedValue;

    qreal fVal1;
    qreal fVal2;
    qreal fVal3;
    qreal fAlpha;
private:
    void startColorTransform(ColorAnimator* const target) const;
    void finishColorTransform(ColorAnimator* const target) const;
    void changeColor(ColorAnimator* const target) const;

    qptr<ColorAnimator> mExclude;
};
#endif // COLORSETTING_H
