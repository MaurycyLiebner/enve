#ifndef EXPRESSIONRANDOMFUNCTION_H
#define EXPRESSIONRANDOMFUNCTION_H
#include "expressionfunctionbase.h"

#include <QRandomGenerator>

class ExpressionRandomFunction : public ExpressionFunctionBase {
public:
    using ExpressionFunctionBase::ExpressionFunctionBase;

    static sptr sCreate(const sptr& value);

    qreal calculateValue(const qreal relFrame) const override;
    FrameRange identicalRange(const qreal relFrame) const override
    { return {qFloor(relFrame), qCeil(relFrame)}; }
};

#endif // EXPRESSIONRANDOMFUNCTION_H
