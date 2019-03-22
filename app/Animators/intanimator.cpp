#include "intanimator.h"

IntAnimator::IntAnimator(const QString& name) : QrealAnimator(name) {
    setNumberDecimals(0);
}

IntAnimator::IntAnimator(const int &iniVal,
                         const int &minVal,
                         const int &maxVal,
                         const int &prefferdStep,
                         const QString &name) :
    QrealAnimator(iniVal, minVal, maxVal, prefferdStep, name) {}

int IntAnimator::getCurrentIntValue() const {
    return qRound(getCurrentBaseValue());
}

int IntAnimator::getCurrentIntValueAtRelFrame(const qreal &relFrame) const {
    return qRound(getBaseValueAtRelFrame(relFrame));
}

int IntAnimator::getCurrentIntEffectiveValueAtRelFrame(const qreal &relFrame) const {
    return qRound(getEffectiveValueAtRelFrame(relFrame));
}

void IntAnimator::setCurrentIntValue(const int &value) {
    setCurrentBaseValue(value);
}

void IntAnimator::setIntValueRange(const int &minVal,
                                   const int &maxVal) {
    setValueRange(minVal, maxVal);
}
