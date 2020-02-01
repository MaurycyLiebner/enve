#include "expressionplainvalue.h"

ExpressionPlainValue::ExpressionPlainValue(const qreal value) :
    mValue(value) { updateValue(); }

using sptr = QSharedPointer<ExpressionPlainValue>;
sptr ExpressionPlainValue::sCreate(const qreal value) {
    return sptr(new ExpressionPlainValue(value));
}
