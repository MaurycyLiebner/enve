#include "brushsettings.h"

BrushSettings::BrushSettings() : ComplexAnimator("brush settings") {
    ca_addChildAnimator(mPressureCurve);
    ca_addChildAnimator(mWidthCurve);
    ca_addChildAnimator(mTimeCurve);

    mPressureCurve->setCurrentValue(qCubicSegment1D(1));
    mWidthCurve->setCurrentValue(qCubicSegment1D(5));
    mTimeCurve->setCurrentValue(qCubicSegment1D(1));
}
