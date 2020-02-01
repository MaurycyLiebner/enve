#ifndef EXPRESSIONSOURCEVALUE_H
#define EXPRESSIONSOURCEVALUE_H
#include "expressionsourcebase.h"

class ExpressionSourceValue : public ExpressionSourceBase {
public:
    ExpressionSourceValue(QrealAnimator* const parent);

    static sptr sCreate(QrealAnimator * const parent);

    qreal calculateValue(const qreal relFrame) const override;
    FrameRange identicalRange(const qreal relFrame) const override;
    QString toString() const override
    { return "$value"; }
};

#endif // EXPRESSIONSOURCEVALUE_H
