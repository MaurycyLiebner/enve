#ifndef QPOINTFANIMATOR_H
#define QPOINTFANIMATOR_H
#include "qrealanimator.h"

class QPointFAnimator
{
public:
    QPointFAnimator();
    QPointF getCurrentValue();
    void setCurrentValue(QPointF val);
private:
    QrealAnimator mXAnimator;
    QrealAnimator mYAnimator;
};

#endif // QPOINTFANIMATOR_H
