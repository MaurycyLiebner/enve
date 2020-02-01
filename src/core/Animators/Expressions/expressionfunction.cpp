#include "expressionfunction.h"

ExpressionFunction::ExpressionFunction(const QString& name,
                                       const std::function<qreal(qreal)>& func,
                                       const sptr& value) :
    ExpressionFunctionBase(name, value), mFunc(func) {}

ExpressionValue::sptr ExpressionFunction::sCreate(
        const QString& name,
        const std::function<qreal(qreal)> &func,
        const sptr &value) {
    return sptr(new ExpressionFunction(name, func, value));
}

qreal ExpressionFunction::calculateValue(const qreal relFrame) const {
    return mFunc(innerValue(relFrame));
}
