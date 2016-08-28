#include "qpointfanimator.h"

QPointFAnimator::QPointFAnimator() : ComplexAnimator()
{

}

QPointF QPointFAnimator::getCurrentValue()
{
    return QPointF(mXAnimator.getCurrentValue(), mYAnimator.getCurrentValue());
}

qreal QPointFAnimator::getXValue()
{
    return mXAnimator.getCurrentValue();
}

qreal QPointFAnimator::getYValue()
{
    return mYAnimator.getCurrentValue();
}

void QPointFAnimator::setCurrentValue(QPointF val)
{
    mXAnimator.setCurrentValue(val.x() );
    mYAnimator.setCurrentValue(val.y() );
}
