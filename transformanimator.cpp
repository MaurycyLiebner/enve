#include "transformanimator.h"

TransformAnimator::TransformAnimator() : ComplexAnimator()
{
    mScaleAnimator.setCurrentValue(QPointF(1., 1.));
    mRotAnimator.setCurrentValue(0.);
    mPosAnimator.setCurrentValue(QPointF(0., 0.) );
}

QMatrix TransformAnimator::getCurrentValue()
{
    QMatrix pivotTrans;
    pivotTrans.translate(-mPivotPoint.x(), -mPivotPoint.y() );
    QMatrix scaleMatrix;
    scaleMatrix.scale(mScaleAnimator.getXValue(), mScaleAnimator.getYValue() );
    QMatrix rotMatrix;
    rotMatrix.rotate(mRotAnimator.getCurrentValue() );
    QMatrix moveMatrix;
    moveMatrix.translate(mPosAnimator.getXValue(), mPosAnimator.getYValue());
    QMatrix unpivotTrans;
    unpivotTrans.translate(mPivotPoint.x(), mPivotPoint.y() );
    return unpivotTrans*moveMatrix*rotMatrix*scaleMatrix*pivotTrans;
}
