#ifndef EXPRESSIONSOURCEBASE_H
#define EXPRESSIONSOURCEBASE_H
#include "expressionvalue.h"
#include "conncontextptr.h"

class QrealAnimator;

class ExpressionSourceBase : public ExpressionValue {
public:
    ExpressionSourceBase(QrealAnimator* const parent);

    bool isPlainValue() const override
    { return false; }
    bool isValid() const override;
    void collapse() override {}
protected:
    ConnContext& setSource(QrealAnimator* const source);
    QrealAnimator* parent() const;
    QrealAnimator* source() const;
private:
    const QPointer<QrealAnimator> mParent;
    ConnContextQPtr<QrealAnimator> mSource;
};

#endif // EXPRESSIONSOURCEBASE_H
