#include "boolanimator.h"

BoolAnimator::BoolAnimator(const QString &name) : IntAnimator(name) {
    setIntValueRange(0, 1);
}

bool BoolAnimator::getCurrentBoolValue() {
    return getCurrentIntValue() == 1;
}

void BoolAnimator::setCurrentBoolValue(const bool bT) {
    setCurrentIntValue((bT ? 1 : 0));
}
