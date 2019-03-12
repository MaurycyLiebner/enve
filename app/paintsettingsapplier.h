#ifndef PAINTSETTINGSAPPLIER_H
#define PAINTSETTINGSAPPLIER_H
#include "smartPointers/sharedpointerdefs.h"
#include "colorsetting.h"
#include "Segments/qcubicsegment1d.h"
#include "GUI/BrushWidgets/brushwidget.h"
#include <QPainter>
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

    void apply(PathBox * const box, const bool &applyAll = true) const;

    void applyColorSetting(ColorAnimator * const animator) const;

    bool targetsFill() const { return mTargetFillSettings; }
private:
    bool mTargetFillSettings;
    bool mLinearGradient = true;
    PaintType mPaintType;
    ColorSetting mColorSetting;
    qptr<Gradient> mGradient;
};

class StrokeSettingsApplier {
public:
    StrokeSettingsApplier() {}
    StrokeSettingsApplier(const ColorSetting &colorSetting) :
        mPaintSettings(true, colorSetting) {}

    StrokeSettingsApplier(const PaintType& paintType) :
        mPaintSettings(true, paintType) {}

    StrokeSettingsApplier(const ColorSetting &colorSetting,
                          const PaintType& paintType) :
        mPaintSettings(true, colorSetting, paintType) {}

    StrokeSettingsApplier(const bool &linearGradient,
                          Gradient* const gradient) :
        mPaintSettings(true, linearGradient, gradient) {}

    StrokeSettingsApplier(const ColorSetting &colorSetting,
                          const qreal &width,
                          SimpleBrushWrapper * const brush,
                          const qCubicSegment1D &widthCurve,
                          const qCubicSegment1D &pressureCurve,
                          const qCubicSegment1D &timeCurve);

    void apply(PathBox * const box, const bool& applyAll = true) const;
private:
    PaintSettingsApplier mPaintSettings;
    QPainter::CompositionMode mOutlineCompositionMode =
            QPainter::CompositionMode_Source;
    qreal mWidth;

    stdptr<SimpleBrushWrapper> mStrokeBrush;
    qCubicSegment1D mWidthCurve;
    qCubicSegment1D mPressureCurve;
    qCubicSegment1D mTimeCurve;
};

#endif // PAINTSETTINGSAPPLIER_H
