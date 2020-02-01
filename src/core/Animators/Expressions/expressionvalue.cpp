#include "expressionvalue.h"

ExpressionValue::ExpressionValue(const bool needsBrackets) :
    mNeedsBrackets(needsBrackets) {}

bool ExpressionValue::setRelFrame(const qreal relFrame) {
    setRelFrameValue(relFrame);
    return updateValue();
}

qreal ExpressionValue::value(const qreal relFrame) const {
    const auto idRange = identicalRange(mRelFrame);
    const bool inRange = idRange.inRange(relFrame);
    if(inRange) return mCurrentValue;
    return calculateValue(relFrame);
}

bool ExpressionValue::updateValue() {
    const qreal newValue = calculateValue(mRelFrame);
    if(isZero4Dec(mCurrentValue - newValue)) return false;
    mCurrentValue = newValue;
    emit currentValueChanged(mCurrentValue);
    return true;
}
