#include "transformanimator.h"
#include <QDebug>

TransformAnimator::TransformAnimator() : ComplexAnimator()
{
    mScaleAnimator.setCurrentValue(QPointF(1., 1.));
    mRotAnimator.setCurrentValue(0.);
    mPosAnimator.setCurrentValue(QPointF(0., 0.) );
}

void TransformAnimator::rotateRelativeToSavedValue(qreal rotRel) {
    mRotAnimator.retrieveSavedValue();
    mRotAnimator.incCurrentValue(rotRel);
}

void TransformAnimator::rotateRelativeToSavedValue(qreal rotRel, QPointF pivot) {
    QMatrix matrix;
    matrix.translate(pivot.x(),
                     pivot.y());
    matrix.rotate(rotRel);
    matrix.translate(-pivot.x() + mPosAnimator.getSavedXValue(),
                     -pivot.y() + mPosAnimator.getSavedYValue() );
    rotateRelativeToSavedValue(rotRel);
    mPosAnimator.setCurrentValue(QPointF(matrix.dx(), matrix.dy()) );
}

void TransformAnimator::translate(qreal dX, qreal dY)
{
    mPosAnimator.incCurrentValue(dX, dY);
}

void TransformAnimator::scale(qreal sx, qreal sy)
{
    mScaleAnimator.retrieveSavedValue();
    mScaleAnimator.multCurrentValue(sx, sy);
}

void TransformAnimator::scale(qreal sx, qreal sy, QPointF pivot)
{
    QMatrix matrix;
    matrix.translate(pivot.x(),
                     pivot.y());
    matrix.scale(sx, sy);
    matrix.translate(-pivot.x() + mPosAnimator.getSavedXValue(),
                     -pivot.y() + mPosAnimator.getSavedYValue() );
    scale(sx, sy);
    mPosAnimator.setCurrentValue(QPointF(matrix.dx(), matrix.dy()) );
}

void TransformAnimator::saveCurrentValue()
{
    mPosAnimator.saveCurrentValue();
    mScaleAnimator.saveCurrentValue();
    mRotAnimator.saveCurrentValue();
}

void TransformAnimator::retrieveSavedValue()
{
    mPosAnimator.retrieveSavedValue();
    mScaleAnimator.retrieveSavedValue();
    mRotAnimator.retrieveSavedValue();
}

void TransformAnimator::finishTransform()
{
    mPosAnimator.finishTransform();
    mScaleAnimator.finishTransform();
    mRotAnimator.finishTransform();
}

qreal TransformAnimator::getYScale()
{
    return mScaleAnimator.getYValue();
}

qreal TransformAnimator::getXScale()
{
    return mScaleAnimator.getXValue();
}

void TransformAnimator::reset()
{
    mScaleAnimator.setCurrentValue(QPointF(1., 1.) );
    mRotAnimator.setCurrentValue(0.);
    mPosAnimator.setCurrentValue(QPointF(0., 0.) );
}

void TransformAnimator::setPivot(qreal x, qreal y, bool saveUndoRedo)
{
    setPivot(QPointF(x, y), saveUndoRedo );
}

void TransformAnimator::setPivot(QPointF point, bool saveUndoRedo)
{
    if(saveUndoRedo) {
//        addUndoRedo(new SetPivotRelPosUndoRedo(this, mRelPivotPos, relPos,
//                                               mPivotChanged, pivotChanged));
    }
    mPivotPoint = point;
}

QPointF TransformAnimator::getPivot()
{
    return mPivotPoint;
}

qreal TransformAnimator::dx()
{
    return mPosAnimator.getXValue();
}

qreal TransformAnimator::dy()
{
    return mPosAnimator.getYValue();
}

qreal TransformAnimator::rot()
{
    return mRotAnimator.getCurrentValue();
}

qreal TransformAnimator::xScale()
{
    return mScaleAnimator.getXValue();
}

qreal TransformAnimator::yScale()
{
    return mScaleAnimator.getYValue();
}

QPointF TransformAnimator::pos()
{
    return mPosAnimator.getCurrentValue();
}

QMatrix TransformAnimator::getCurrentValue()
{
    QMatrix matrix;
    matrix.translate(mPivotPoint.x() + mPosAnimator.getXValue(),
                     mPivotPoint.y() + mPosAnimator.getYValue());
    matrix.scale(mScaleAnimator.getXValue(), mScaleAnimator.getYValue() );
    matrix.rotate(mRotAnimator.getCurrentValue() );
    matrix.translate(-mPivotPoint.x(),
                     -mPivotPoint.y());
    return matrix;
}
