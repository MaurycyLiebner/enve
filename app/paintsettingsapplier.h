#ifndef PAINTSETTINGSAPPLIER_H
#define PAINTSETTINGSAPPLIER_H
#include "smartPointers/sharedpointerdefs.h"
#include "colorsetting.h"
class ColorAnimator;
class Gradient;
class PathBox;
enum PaintType : short;

class PaintSettingsApplier {
public:
    PaintSettingsApplier() {}
    PaintSettingsApplier(const bool &targetFillSettings,
                         const ColorSetting &colorSetting);

    PaintSettingsApplier(const bool &targetFillSettings,
                         const PaintType& paintType);

    PaintSettingsApplier(const bool &targetFillSettings,
                         const ColorSetting &colorSetting,
                         const PaintType& paintType);

    PaintSettingsApplier(const bool &targetFillSettings,
                         const bool &linearGradient,
                         Gradient* const gradient);

    void apply(PathBox * const box) const;

    void applyColorSetting(ColorAnimator *animator) const;

    bool targetsFill() const { return mTargetFillSettings; }
private:
    bool mTargetFillSettings;
    bool mLinearGradient = true;
    PaintType mPaintType;
    ColorSetting mColorSetting;
    qptr<Gradient> mGradient;
};



#endif // PAINTSETTINGSAPPLIER_H
