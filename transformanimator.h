#ifndef TRANSFORMANIMATOR_H
#define TRANSFORMANIMATOR_H
#include "complexanimator.h"
#include "qpointfanimator.h"

class TransformAnimator : public ComplexAnimator
{
public:
    TransformAnimator();

    QMatrix getCurrentValue();
private:
    QPointF mPivotPoint;
    QPointFAnimator mPosAnimator;
    QPointFAnimator mScaleAnimator;
    QrealAnimator mRotAnimator;
};

#endif // TRANSFORMANIMATOR_H
