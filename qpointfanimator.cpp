#include "qpointfanimator.h"
#include "boxeslist.h"

QPointFAnimator::QPointFAnimator() : ComplexAnimator()
{
    mXAnimator.setName("x");
    mXAnimator.setParentAnimator(this);
    mYAnimator.setName("y");
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

qreal QPointFAnimator::getBoxesListHeight()
{
    if(mBoxesListDetailVisible) {
        return 3*LIST_ITEM_HEIGHT;
    } else {
        return LIST_ITEM_HEIGHT;
    }
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

    updateKeyOnCurrrentFrame();
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

void QPointFAnimator::drawBoxesList(QPainter *p,
                                      qreal drawX, qreal drawY,
                                      qreal pixelsPerFrame,
                                      int startFrame, int endFrame)
{
    QrealAnimator::drawBoxesList(p, drawX, drawY,
                                 pixelsPerFrame, startFrame, endFrame);
    if(mBoxesListDetailVisible) {
        drawX += LIST_ITEM_CHILD_INDENT;
        drawY += LIST_ITEM_HEIGHT;
        mXAnimator.drawBoxesList(p, drawX, drawY,
                                pixelsPerFrame,
                                startFrame, endFrame);
        drawY += mXAnimator.getBoxesListHeight();

        mYAnimator.drawBoxesList(p, drawX, drawY,
                                pixelsPerFrame,
                                startFrame, endFrame);
        drawY += mYAnimator.getBoxesListHeight();
    }
}
QrealKey *QPointFAnimator::getKeyAtPos(qreal relX, qreal relY,
                                     int minViewedFrame,
                                     qreal pixelsPerFrame) {
    if(relY <= LIST_ITEM_HEIGHT) {
        return QrealAnimator::getKeyAtPos(relX, relY,
                                          minViewedFrame, pixelsPerFrame);
    } else if(mBoxesListDetailVisible) {
        if(relY <= 2*LIST_ITEM_HEIGHT) {
            return mXAnimator.getKeyAtPos(relX, relY,
                                          minViewedFrame, pixelsPerFrame);
        } else if(relY <= 3*LIST_ITEM_HEIGHT) {
            return mYAnimator.getKeyAtPos(relX, relY,
                                          minViewedFrame, pixelsPerFrame);
        }
    }
    return NULL;
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
