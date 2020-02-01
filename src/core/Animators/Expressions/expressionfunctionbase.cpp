#include "expressionfunctionbase.h"
#include "expressionplainvalue.h"

ExpressionFunctionBase::ExpressionFunctionBase(
        const QString &name, const sptr& value) :
    ExpressionValue(false), mName(name), mValue(value) {
    connect(mValue.get(), &ExpressionValue::currentValueChanged,
            this, &ExpressionValue::updateValue);
    connect(mValue.get(), &ExpressionValue::relRangeChanged,
            this, &ExpressionValue::relRangeChanged);
}

void ExpressionFunctionBase::collapse() {
    mValue->collapse();
    if(mValue->isPlainValue()) {
        mValue = ExpressionPlainValue::sCreate(mValue->currentValue());
    }
}

bool ExpressionFunctionBase::isPlainValue() const
{ return mValue->isPlainValue(); }

bool ExpressionFunctionBase::setRelFrame(const qreal relFrame) {
    setRelFrameValue(relFrame);
    return mValue->setRelFrame(relFrame);
}
