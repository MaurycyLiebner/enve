#include "qpointfanimator.h"

QPointFAnimator::QPointFAnimator() : ComplexAnimator()
{
    mXAnimator.setParentAnimator(this);
    mYAnimator.setParentAnimator(this);
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

void QPointFAnimator::incCurrentValue(qreal x, qreal y)
{
    mXAnimator.incCurrentValue(x);
    mYAnimator.incCurrentValue(y);
}

void QPointFAnimator::incAllValues(qreal x, qreal y) {
    mXAnimator.incAllValues(x);
    mYAnimator.incAllValues(y);
}

void QPointFAnimator::multCurrentValue(qreal sx, qreal sy)
{
    mXAnimator.multCurrentValue(sx);
    mYAnimator.multCurrentValue(sy);
}

void QPointFAnimator::startTransform()
{
    mXAnimator.startTransform();
    mYAnimator.startTransform();
}

void QPointFAnimator::setConnectedToMainWindow(ConnectedToMainWindow *connected)
{
    QrealAnimator::setConnectedToMainWindow(connected);
    mXAnimator.setConnectedToMainWindow(connected);
    mYAnimator.setConnectedToMainWindow(connected);
}

void QPointFAnimator::setUpdater(AnimatorUpdater *updater)
{
    QrealAnimator::setUpdater(updater);

    mXAnimator.setUpdater(updater);
    mYAnimator.setUpdater(updater);
}

void QPointFAnimator::setFrame(int frame)
{
    QrealAnimator::setFrame(frame);
    mXAnimator.setFrame(frame);
    mYAnimator.setFrame(frame);
}

void QPointFAnimator::sortKeys()
{
    QrealAnimator::sortKeys();
    mXAnimator.sortKeys();
    mYAnimator.sortKeys();
}

void QPointFAnimator::updateKeysPath()
{
    QrealAnimator::updateKeysPath();
    mXAnimator.updateKeysPath();
    mYAnimator.updateKeysPath();
}

void QPointFAnimator::finishTransform(bool record)
{
    mConnectedToMainWindow->startNewUndoRedoSet();

    mXAnimator.finishTransform(record);
    mYAnimator.finishTransform(record);

    mConnectedToMainWindow->finishUndoRedoSet();
}

void QPointFAnimator::cancelTransform() {
    mXAnimator.cancelTransform();
    mYAnimator.cancelTransform();
}

void QPointFAnimator::retrieveSavedValue()
{
    mXAnimator.retrieveSavedValue();
    mYAnimator.retrieveSavedValue();
}

QPointF QPointFAnimator::getSavedValue()
{
    return QPointF(mXAnimator.getSavedValue(), mYAnimator.getSavedValue() );
}

qreal QPointFAnimator::getSavedXValue()
{
    return mXAnimator.getSavedValue();
}

qreal QPointFAnimator::getSavedYValue()
{
    return mYAnimator.getSavedValue();
}
