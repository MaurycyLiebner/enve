#include "transformanimator.h"
#include "undoredo.h"
#include "boxeslist.h"
#include <QDebug>

TransformAnimator::TransformAnimator() : ComplexAnimator()
{
    setName("transformation");
    mScaleAnimator.setName("scale");
    mScaleAnimator.setCurrentValue(QPointF(1., 1.));
    mRotAnimator.setName("rot");
    mRotAnimator.setCurrentValue(0.);
    mPosAnimator.setName("pos");
    mPosAnimator.setCurrentValue(QPointF(0., 0.) );
    mPivotAnimator.setName("pivot");
    mPivotAnimator.setCurrentValue(QPointF(0., 0.) );
    mOpacityAnimator.setName("opacity");
    mOpacityAnimator.setValueRange(0., 100.);
    mOpacityAnimator.setCurrentValue(100.);

    addChildAnimator(&mPosAnimator);
    addChildAnimator(&mRotAnimator);
    addChildAnimator(&mScaleAnimator);
    mScaleAnimator.setPrefferedValueStep(0.05);
    addChildAnimator(&mPivotAnimator);
    addChildAnimator(&mOpacityAnimator);
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

void TransformAnimator::startOpacityTransform() {
    mOpacityAnimator.startTransform();
}

void TransformAnimator::setOpacity(qreal newOpacity) {
    mOpacityAnimator.setCurrentValue(newOpacity);
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

void TransformAnimator::setPivot(QPointF point, bool finish)
{
    QMatrix currentMatrix;
    qreal pivotX = mPivotAnimator.getXValue();
    qreal pivotY = mPivotAnimator.getYValue();
    currentMatrix.translate(pivotX + mPosAnimator.getXValue(),
                     pivotY + mPosAnimator.getYValue());
    currentMatrix.scale(mScaleAnimator.getXValue(),
                        mScaleAnimator.getYValue() );
    currentMatrix.rotate(mRotAnimator.getCurrentValue() );
    currentMatrix.translate(-pivotX,
                            -pivotY);

    QMatrix futureMatrix;
    futureMatrix.translate(point.x() + mPosAnimator.getXValue(),
                     point.y() + mPosAnimator.getYValue());
    futureMatrix.scale(mScaleAnimator.getXValue(),
                        mScaleAnimator.getYValue() );
    futureMatrix.rotate(mRotAnimator.getCurrentValue() );
    futureMatrix.translate(-point.x(),
                            -point.y());

    mPosAnimator.incAllValues(currentMatrix.dx() - futureMatrix.dx(),
                                 currentMatrix.dy() - futureMatrix.dy());

    mPivotAnimator.setCurrentValue(point, finish);

    callUpdater();
}

QPointF TransformAnimator::getPivot()
{
    return mPivotAnimator.getCurrentValue();
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
    return mPivotAnimator.getXValue();
}

qreal TransformAnimator::getPivotY()
{
    return mPivotAnimator.getYValue();
}

qreal TransformAnimator::getOpacity()
{
    return mOpacityAnimator.getCurrentValue();
}

QMatrix TransformAnimator::getCurrentValue()
{
    QMatrix matrix;
    qreal pivotX = mPivotAnimator.getXValue();
    qreal pivotY = mPivotAnimator.getYValue();
    matrix.translate(pivotX + mPosAnimator.getXValue(),
                     pivotY + mPosAnimator.getYValue());
    matrix.scale(mScaleAnimator.getXValue(), mScaleAnimator.getYValue() );
    matrix.rotate(mRotAnimator.getCurrentValue() );
    matrix.translate(-pivotX,
                     -pivotY);
    return matrix;
}
