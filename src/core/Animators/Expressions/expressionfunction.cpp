#include "expressionfunction.h"
#include "expressionplainvalue.h"

ExpressionFunction::ExpressionFunction(const std::function<qreal(qreal)>& func,
                                       const sptr& value) :
    mFunc(func), mValue(value) {
    connect(mValue.get(), &ExpressionValue::currentValueChanged,
            this, &ExpressionValue::updateValue);
    connect(mValue.get(), &ExpressionValue::relRangeChanged,
            this, &ExpressionValue::relRangeChanged);
}

ExpressionValue::sptr ExpressionFunction::sCreate(
        const std::function<qreal(qreal)> &func,
        const sptr &value) {
    return sptr(new ExpressionFunction(func, value));
}

void ExpressionFunction::collapse() {
    mValue->collapse();
    if(mValue->isPlainValue()) {
        mValue = ExpressionPlainValue::sCreate(mValue->currentValue());
    }
}

bool ExpressionFunction::isPlainValue() const
{ return mValue->isPlainValue(); }

bool ExpressionFunction::setRelFrame(const qreal relFrame) {
    setRelFrameValue(relFrame);
    return mValue->setRelFrame(relFrame);
}
