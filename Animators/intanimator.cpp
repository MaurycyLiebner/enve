#include "intanimator.h"

IntAnimator::IntAnimator() : QrealAnimator() {
    setNumberDecimals(0);
}

int IntAnimator::getCurrentIntValue() const {
    return qRound(getCurrentValue());
}

void IntAnimator::setCurrentIntValue(int value, bool finish) {
    setCurrentValue(value, finish);
}
