#include "smartpathkey.h"

SmartPathKey::SmartPathKey(const SmartPath &value, const int relFrame,
                           Animator * const parentAnimator) :
    InterpolationKeyT<SmartPath>(relFrame, parentAnimator) {
    assignValue(value);
}

SmartPathKey::SmartPathKey(Animator * const parentAnimator) :
    InterpolationKeyT<SmartPath>(parentAnimator) {}

void SmartPathKey::save() {
    mValue.save();
}

void SmartPathKey::restore() {
    mValue.restore();
}

void SmartPathKey::assignValue(const SmartPath &value) {
    mValue.deepCopy(value);
}
