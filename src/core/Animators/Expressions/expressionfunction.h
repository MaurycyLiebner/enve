#ifndef EXPRESSIONFUNCTION_H
#define EXPRESSIONFUNCTION_H
#include "expressionfunctionbase.h"

class ExpressionFunction : public ExpressionFunctionBase {
public:
    ExpressionFunction(const QString& name,
                       const std::function<qreal(qreal)>& func,
                       const sptr& value);

    static sptr sCreate(const QString& name,
                        const std::function<qreal(qreal)>& func,
                        const sptr& value);

    qreal calculateValue(const qreal relFrame) const override;
private:
    const std::function<qreal(qreal)> mFunc;
};

#endif // EXPRESSIONFUNCTION_H
