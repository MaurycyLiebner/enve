#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H
#include "expressionvalue.h"

class QrealAnimator;

namespace ExpressionParser {
    QSharedPointer<ExpressionValue> parse(
            QString exp, QrealAnimator* const parent);
};

#endif // EXPRESSIONPARSER_H
