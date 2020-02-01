#include "expressionoperator.h"
#include "expressionplainvalue.h"

ExpressionOperator::ExpressionOperator(
        const std::function<qreal(qreal, qreal)> &func,
        const sptr &value1, const sptr &value2) :
    mFunc(func), mValue1(value1), mValue2(value2) {
    connect(mValue1.get(), &ExpressionValue::currentValueChanged,
            this, &ExpressionValue::updateValue);
    connect(mValue1.get(), &ExpressionValue::relRangeChanged,
            this, &ExpressionValue::relRangeChanged);
    connect(mValue2.get(), &ExpressionValue::currentValueChanged,
            this, &ExpressionValue::updateValue);
    connect(mValue2.get(), &ExpressionValue::relRangeChanged,
            this, &ExpressionValue::relRangeChanged);
}

ExpressionValue::sptr ExpressionOperator::sCreate(
        const std::function<qreal (qreal, qreal)> &func,
        const sptr &value1, const sptr &value2) {
    return sptr(new ExpressionOperator(func, value1, value2));
}

void ExpressionOperator::collapse() {
    mValue1->collapse();
    mValue2->collapse();
    if(mValue1->isPlainValue()) {
        mValue1 = ExpressionPlainValue::sCreate(mValue1->currentValue());
    }
    if(mValue2->isPlainValue()) {
        mValue2 = ExpressionPlainValue::sCreate(mValue2->currentValue());
    }
}

bool ExpressionOperator::isPlainValue() const {
    return mValue1->isPlainValue() &&
           mValue2->isPlainValue();
}

bool ExpressionOperator::isValid() const {
    return mValue1->isValid() &&
           mValue2->isValid();
}

FrameRange ExpressionOperator::identicalRange(const qreal relFrame) const {
    const auto id1 = mValue1->identicalRange(relFrame);
    const auto id2 = mValue2->identicalRange(relFrame);
    return id1*id2;
}

bool ExpressionOperator::setRelFrame(const qreal relFrame) {
    setRelFrameValue(relFrame);
    const bool changed1 = mValue1->setRelFrame(relFrame);
    const bool changed2 = mValue2->setRelFrame(relFrame);
    return changed1 || changed2;
}
