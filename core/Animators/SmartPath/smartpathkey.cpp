#include "smartpathkey.h"

SmartPathKey::SmartPathKey(const SmartPath &value, const int &relFrame, Animator * const parentAnimator) :
    InterpolationKeyT<SmartPath>(value, relFrame, parentAnimator) {}

SmartPathKey::SmartPathKey(Animator * const parentAnimator) :
    InterpolationKeyT<SmartPath>(parentAnimator) {}

void SmartPathKey::save() {
    mValue.save();
}

void SmartPathKey::restore() {
    mValue.restore();
}

void SmartPathKey::updateAfterPrevKeyChanged(Key * const prevKey) {
    const auto spk = static_cast<SmartPathKey*>(prevKey);
    if(prevKey) mValue.setPrev(&spk->getValue());
    else mValue.setPrev(nullptr);
}

void SmartPathKey::updateAfterNextKeyChanged(Key * const nextKey) {
    const auto spk = static_cast<SmartPathKey*>(nextKey);
    if(nextKey) mValue.setNext(&spk->getValue());
    else mValue.setNext(nullptr);
}

void SmartPathKey::assignValue(const SmartPath &value) {
    mValue.assign(value);
}
