#include "expressionplainvalue.h"

ExpressionPlainValue::ExpressionPlainValue(const qreal value) :
    ExpressionPlainValue(QString::number(value), value) {}

ExpressionPlainValue::ExpressionPlainValue(const QString &string,
                                           const qreal value) :
    ExpressionValue(false),
    mString(string), mValue(value) { updateValue(); }

using sptr = QSharedPointer<ExpressionPlainValue>;
sptr ExpressionPlainValue::sCreate(
        const QString &string, const qreal value) {
    return sptr(new ExpressionPlainValue(string, value));
}

sptr ExpressionPlainValue::sCreate(const qreal value) {
    return sptr(new ExpressionPlainValue(value));
}
