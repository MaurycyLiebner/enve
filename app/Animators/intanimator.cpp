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
    return qRound(qra_getCurrentValue());
}

int IntAnimator::getCurrentIntValueAtRelFrame(const int &relFrame) const {
    return qRound(getCurrentValueAtRelFrame(relFrame));
}

int IntAnimator::getCurrentIntValueAtRelFrameF(const qreal &relFrame) const {
    return qRound(getCurrentValueAtRelFrameF(relFrame));
}

int IntAnimator::getCurrentIntEffectiveValueAtRelFrameF(const qreal &relFrame) const {
    return qRound(getCurrentEffectiveValueAtRelFrameF(relFrame));
}

void IntAnimator::setCurrentIntValue(const int &value) {
    qra_setCurrentValue(value);
}

void IntAnimator::setIntValueRange(const int &minVal,
                                   const int &maxVal) {
    qra_setValueRange(minVal, maxVal);
}
