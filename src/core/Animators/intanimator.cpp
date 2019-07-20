#include "intanimator.h"

IntAnimator::IntAnimator(const QString& name) : QrealAnimator(name) {
    setNumberDecimals(0);
}

IntAnimator::IntAnimator(const int iniVal,
                         const int minVal,
                         const int maxVal,
                         const int prefferdStep,
                         const QString &name) :
    QrealAnimator(iniVal, minVal, maxVal, prefferdStep, name) {}

int IntAnimator::getCurrentIntValue() const {
    return qRound(getCurrentBaseValue());
}

int IntAnimator::getBaseIntValue(const qreal relFrame) const {
    return qRound(getBaseValue(relFrame));
}

int IntAnimator::getEffectiveIntValue(const qreal relFrame) const {
    return qRound(getEffectiveValue(relFrame));
}

void IntAnimator::setCurrentIntValue(const int value) {
    setCurrentBaseValue(value);
}

void IntAnimator::setIntValueRange(const int minVal,
                                   const int maxVal) {
    setValueRange(minVal, maxVal);
}
