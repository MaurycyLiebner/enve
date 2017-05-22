#include "intanimator.h"

IntAnimator::IntAnimator() : QrealAnimator() {
    setNumberDecimals(0);
}

int IntAnimator::getCurrentIntValue() const {
    return qRound(qra_getCurrentValue());
}

void IntAnimator::setCurrentIntValue(const int &value,
                                     const bool &finish) {
    qra_setCurrentValue(value, finish);
}
