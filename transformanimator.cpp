#include "transformanimator.h"
#include "undoredo.h"
#include <QDebug>

TransformAnimator::TransformAnimator() : ComplexAnimator()
{
    mScaleAnimator.setCurrentValue(QPointF(1., 1.));
    mRotAnimator.setCurrentValue(0.);
    mPosAnimator.setCurrentValue(QPointF(0., 0.) );

    mScaleAnimator.setParentAnimator(this);
    mRotAnimator.setParentAnimator(this);
    mPosAnimator.setParentAnimator(this);
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

void TransformAnimator::setScale(qreal sx, qreal sy)
{
    mScaleAnimator.setCurrentValue(QPointF(sx, sy) );
}

void TransformAnimator::setPosition(qreal x, qreal y)
{
    mPosAnimator.setCurrentValue(QPointF(x, y) );
}

void TransformAnimator::setRotation(qreal rot)
{
    mRotAnimator.setCurrentValue(rot);
}

void TransformAnimator::startTransform()
{
    startRotTransform();
    startPosTransform();
    startScaleTransform();
}

void TransformAnimator::startRotTransform()
{
    mRotAnimator.startTransform();
}

void TransformAnimator::startPosTransform() {
    mPosAnimator.startTransform();
}

void TransformAnimator::startScaleTransform() {
    mScaleAnimator.startTransform();
}

void TransformAnimator::setConnectedToMainWindow(ConnectedToMainWindow *connected)
{
    QrealAnimator::setConnectedToMainWindow(connected);

    mPosAnimator.setConnectedToMainWindow(connected);
    mRotAnimator.setConnectedToMainWindow(connected);
    mScaleAnimator.setConnectedToMainWindow(connected);
}

void TransformAnimator::setUpdater(AnimatorUpdater *updater)
{
    QrealAnimator::setUpdater(updater);

    mPosAnimator.setUpdater(updater);
    mRotAnimator.setUpdater(updater);
    mScaleAnimator.setUpdater(updater);
}

void TransformAnimator::setFrame(int frame)
{
    QrealAnimator::setFrame(frame);

    mPosAnimator.setFrame(frame);
    mRotAnimator.setFrame(frame);
    mScaleAnimator.setFrame(frame);
}

void TransformAnimator::retrieveSavedValue()
{
    mPosAnimator.retrieveSavedValue();
    mScaleAnimator.retrieveSavedValue();
    mRotAnimator.retrieveSavedValue();
}

void TransformAnimator::finishTransform(bool record)
{
    mConnectedToMainWindow->startNewUndoRedoSet();

    mPosAnimator.finishTransform(record);
    mScaleAnimator.finishTransform(record);
    mRotAnimator.finishTransform(record);

    mConnectedToMainWindow->finishUndoRedoSet();
}

void TransformAnimator::cancelTransform()
{
    mPosAnimator.cancelTransform();
    mScaleAnimator.cancelTransform();
    mRotAnimator.cancelTransform();
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

void TransformAnimator::setPivot(qreal x, qreal y)
{
    setPivot(QPointF(x, y) );
}

void TransformAnimator::setPivot(QPointF point)
{
    QMatrix currentMatrix;
    currentMatrix.translate(mPivotPoint.x() + mPosAnimator.getXValue(),
                     mPivotPoint.y() + mPosAnimator.getYValue());
    currentMatrix.scale(mScaleAnimator.getXValue(),
                        mScaleAnimator.getYValue() );
    currentMatrix.rotate(mRotAnimator.getCurrentValue() );
    currentMatrix.translate(-mPivotPoint.x(),
                            -mPivotPoint.y());

    QMatrix futureMatrix;
    futureMatrix.translate(point.x() + mPosAnimator.getXValue(),
                     point.y() + mPosAnimator.getYValue());
    futureMatrix.scale(mScaleAnimator.getXValue(),
                        mScaleAnimator.getYValue() );
    futureMatrix.rotate(mRotAnimator.getCurrentValue() );
    futureMatrix.translate(-point.x(),
                            -point.y());

    mPosAnimator.incCurrentValue(currentMatrix.dx() - futureMatrix.dx(),
                                 currentMatrix.dy() - futureMatrix.dy());

    mPivotPoint = point;

    mUpdater->update();
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

qreal TransformAnimator::getPivotX()
{
    return mPivotPoint.x();
}

qreal TransformAnimator::getPivotY()
{
    return mPivotPoint.y();
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
