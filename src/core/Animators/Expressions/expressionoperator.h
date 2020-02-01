#ifndef EXPRESSIONOPERATOR_H
#define EXPRESSIONOPERATOR_H
#include "expressionvalue.h"

class ExpressionOperator : public ExpressionValue {
public:
    ExpressionOperator(const bool childrenNeedBrackets,
                       const bool needsBrackets, const QString& symbol,
                       const std::function<qreal(qreal, qreal)>& func,
                       const sptr &value1, const sptr &value2);

    static sptr sCreate(const bool childrenNeedBrackets,
                        const bool needsBrackets, const QString& name,
                        const std::function<qreal(qreal, qreal)>& func,
                        const sptr& value1, const sptr& value2);

    qreal calculateValue(const qreal relFrame) const override
    { return mFunc(value1(relFrame), value2(relFrame)); }
    void collapse() override;
    bool isPlainValue() const override;
    bool isValid() const override;
    FrameRange identicalRange(const qreal relFrame) const override;
    bool setRelFrame(const qreal relFrame) override;
    QString toString() const override;
private:
    qreal value1(const qreal relFrame) const
    { return mValue1->value(relFrame); }
    qreal value2(const qreal relFrame) const
    { return mValue2->value(relFrame); }

    const bool mChildrenNeedBrackets;
    const QString mSymbol;
    const std::function<qreal(qreal, qreal)> mFunc;
    sptr mValue1;
    sptr mValue2;
};

#endif // EXPRESSIONOPERATOR_H
