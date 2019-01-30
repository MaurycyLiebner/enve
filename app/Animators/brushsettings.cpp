#include "brushsettings.h"

BrushSettings::BrushSettings() : ComplexAnimator("brush settings") {
    ca_addChildAnimator(mPressureCurve);
    ca_addChildAnimator(mWidthCurve);
    ca_addChildAnimator(mTimeCurve);
}
