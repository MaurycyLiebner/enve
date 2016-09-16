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

    mScaleAnimator.setParentAnimator(this);
    mRotAnimator.setParentAnimator(this);
    mPosAnimator.setParentAnimator(this);
    mPivotAnimator.setParentAnimator(this);
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
    mPivotAnimator.setConnectedToMainWindow(connected);
}

void TransformAnimator::setUpdater(AnimatorUpdater *updater)
{
    QrealAnimator::setUpdater(updater);

    mPosAnimator.setUpdater(updater);
    mRotAnimator.setUpdater(updater);
    mScaleAnimator.setUpdater(updater);
    mPivotAnimator.setUpdater(updater);
}

void TransformAnimator::setFrame(int frame)
{
    QrealAnimator::setFrame(frame);

    mPosAnimator.setFrame(frame);
    mRotAnimator.setFrame(frame);
    mScaleAnimator.setFrame(frame);
    mPivotAnimator.setFrame(frame);
}

void TransformAnimator::sortKeys()
{
    QrealAnimator::sortKeys();
    mPosAnimator.sortKeys();
    mRotAnimator.sortKeys();
    mScaleAnimator.sortKeys();
    mPivotAnimator.sortKeys();
}

void TransformAnimator::updateKeysPath()
{
    QrealAnimator::updateKeysPath();
    mPosAnimator.updateKeysPath();
    mRotAnimator.updateKeysPath();
    mScaleAnimator.updateKeysPath();
    mPivotAnimator.updateKeysPath();
}

qreal TransformAnimator::getBoxesListHeight()
{
    if(mBoxesListDetailVisible) {
        return mPosAnimator.getBoxesListHeight() +
               mRotAnimator.getBoxesListHeight() +
               mScaleAnimator.getBoxesListHeight() +
               mPivotAnimator.getBoxesListHeight() +
               LIST_ITEM_HEIGHT;
    } else {
        return LIST_ITEM_HEIGHT;
    }
}

void TransformAnimator::drawBoxesList(QPainter *p,
                                      qreal drawX, qreal drawY,
                                      qreal pixelsPerFrame,
                                      int startFrame, int endFrame)
{
    QrealAnimator::drawBoxesList(p, drawX, drawY,
                                 pixelsPerFrame, startFrame, endFrame);
    if(mBoxesListDetailVisible) {
        drawX += LIST_ITEM_CHILD_INDENT;
        drawY += LIST_ITEM_HEIGHT;
        mPosAnimator.drawBoxesList(p, drawX, drawY,
                                pixelsPerFrame,
                                startFrame, endFrame);
        drawY += mPosAnimator.getBoxesListHeight();

        mScaleAnimator.drawBoxesList(p, drawX, drawY,
                                pixelsPerFrame,
                                startFrame, endFrame);
        drawY += mScaleAnimator.getBoxesListHeight();

        mRotAnimator.drawBoxesList(p, drawX, drawY,
                                pixelsPerFrame,
                                startFrame, endFrame);
        drawY += mRotAnimator.getBoxesListHeight();

        mPivotAnimator.drawBoxesList(p, drawX, drawY,
                                pixelsPerFrame,
                                startFrame, endFrame);
        drawY += mPivotAnimator.getBoxesListHeight();
    }
}

QrealKey *TransformAnimator::getKeyAtPos(qreal relX, qreal relY,
                                     int minViewedFrame,
                                     qreal pixelsPerFrame) {
    if(relY <= LIST_ITEM_HEIGHT) {
        return QrealAnimator::getKeyAtPos(relX, relY,
                                   minViewedFrame, pixelsPerFrame);
    } else if(mBoxesListDetailVisible) {
        relY -= LIST_ITEM_HEIGHT;
        qreal posHeight = mPosAnimator.getBoxesListHeight();
        if(relY <= posHeight) {
            return mPosAnimator.getKeyAtPos(relX, relY,
                                     minViewedFrame, pixelsPerFrame);
        }
        relY -= posHeight;
        qreal scaleHeight = mScaleAnimator.getBoxesListHeight();
        if(relY <= scaleHeight) {
            return mScaleAnimator.getKeyAtPos(relX, relY,
                                       minViewedFrame, pixelsPerFrame);
        }
        relY -= scaleHeight;
        qreal rotHeight = mRotAnimator.getBoxesListHeight();
        if(relY <= rotHeight) {
            return mRotAnimator.getKeyAtPos(relX, relY,
                                     minViewedFrame, pixelsPerFrame);
        }
        relY -= rotHeight;
        qreal pivotHeight = mPivotAnimator.getBoxesListHeight();
        if(relY <= pivotHeight) {
            return mPivotAnimator.getKeyAtPos(relX, relY,
                                       minViewedFrame, pixelsPerFrame);
        }
    }
    return NULL;
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

    mPivotAnimator.startTransform();
    mPivotAnimator.setCurrentValue(point);
    mPivotAnimator.finishTransform(mConnectedToMainWindow->isRecording());

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
