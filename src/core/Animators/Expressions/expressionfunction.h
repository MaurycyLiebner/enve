#ifndef EXPRESSIONFUNCTION_H
#define EXPRESSIONFUNCTION_H
#include "expressionvalue.h"

class ExpressionFunction : public ExpressionValue {
public:
    ExpressionFunction(const std::function<qreal(qreal)>& func,
                       const sptr& value);

    static sptr sCreate(const std::function<qreal(qreal)>& func,
                        const sptr& value);

    qreal calculateValue(const qreal relFrame) const override
    { return mFunc(mValue->value(relFrame)); }

    void collapse() override;
    bool isPlainValue() const override;

    bool setRelFrame(const qreal relFrame) override;

    bool isValid() const override
    { return mValue->isValid(); }
    FrameRange identicalRange(const qreal relFrame) const override
    { return mValue->identicalRange(relFrame); }
private:
    const std::function<qreal(qreal)> mFunc;
    sptr mValue;
};

#endif // EXPRESSIONFUNCTION_H
