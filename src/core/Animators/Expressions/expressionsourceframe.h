#ifndef EXPRESSIONSOURCEFRAME_H
#define EXPRESSIONSOURCEFRAME_H
#include "expressionsourcebase.h"

class ExpressionSourceFrame : public ExpressionSourceBase {
public:
    ExpressionSourceFrame(QrealAnimator* const parent);

    static sptr sCreate(QrealAnimator * const parent);

    qreal calculateValue(const qreal relFrame) const override;
    FrameRange identicalRange(const qreal relFrame) const override;
    QString toString() const override
    { return "$frame"; }
};

#endif // EXPRESSIONSOURCEFRAME_H
