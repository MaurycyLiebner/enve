#ifndef EXPRESSIONFUNCTIONBASE_H
#define EXPRESSIONFUNCTIONBASE_H
#include "expressionvalue.h"

class ExpressionFunctionBase : public ExpressionValue {
public:
    ExpressionFunctionBase(const QString& name,
                           const sptr& value);

    void collapse() override;
    bool isPlainValue() const override;

    bool setRelFrame(const qreal relFrame) override;

    bool isValid() const override
    { return mValue->isValid(); }
    FrameRange identicalRange(const qreal relFrame) const override
    { return mValue->identicalRange(relFrame); }
    QString toString() const override
    { return mName + "( " + mValue->toString() + " )"; }
protected:
    qreal innerValue(const qreal relFrame) const
    { return mValue->value(relFrame); }
private:
    const QString mName;
    sptr mValue;
};

#endif // EXPRESSIONFUNCTIONBASE_H
