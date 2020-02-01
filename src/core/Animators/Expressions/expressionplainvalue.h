#ifndef EXPRESSIONPLAINVALUE_H
#define EXPRESSIONPLAINVALUE_H

#include "expressionvalue.h"

class ExpressionPlainValue : public ExpressionValue {
public:
    ExpressionPlainValue(const qreal value);

    using sptr = QSharedPointer<ExpressionPlainValue>;
    static sptr sCreate(const qreal value);

    qreal calculateValue(const qreal relFrame) const override
    { Q_UNUSED(relFrame) return mValue; }
    bool isPlainValue() const override { return true; }
    bool isValid() const override { return true; }
    void collapse() override {}
    FrameRange identicalRange(const qreal relFrame) const override
    { Q_UNUSED(relFrame) return FrameRange::EMINMAX; }
private:
    const qreal mValue;
};

#endif // EXPRESSIONPLAINVALUE_H
