#include "brushsettings.h"

BrushSettings::BrushSettings() : ComplexAnimator("brush settings") {
    ca_addChildAnimator(mWidthCurve);
    ca_addChildAnimator(mPressureCurve);
    ca_addChildAnimator(mSpacingCurve);
    ca_addChildAnimator(mTimeCurve);

    mWidthCurve->setCurrentValue(qCubicSegment1D(1));
    mPressureCurve->setCurrentValue(qCubicSegment1D(1));
    mSpacingCurve->setCurrentValue(qCubicSegment1D(1));
    mTimeCurve->setCurrentValue(qCubicSegment1D(1));
}
