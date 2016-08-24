#include "qpointfanimator.h"

QPointFAnimator::QPointFAnimator()
{

}

QPointF QPointFAnimator::getCurrentValue()
{
    return QPointF(mXAnimator.getCurrentValue(), mYAnimator.getCurrentValue());
}

void QPointFAnimator::setCurrentValue(QPointF val)
{
    mXAnimator.setCurrentValue(val.x() );
    mYAnimator.setCurrentValue(val.y() );
}
