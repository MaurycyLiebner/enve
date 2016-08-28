#ifndef QPOINTFANIMATOR_H
#define QPOINTFANIMATOR_H
#include "complexanimator.h"
#include "qrealanimator.h"

class QPointFAnimator : public ComplexAnimator
{
public:
    QPointFAnimator();
    QPointF getCurrentValue();
    qreal getXValue();
    qreal getYValue();
    void setCurrentValue(QPointF val);
private:
    QrealAnimator mXAnimator;
    QrealAnimator mYAnimator;
};

#endif // QPOINTFANIMATOR_H
