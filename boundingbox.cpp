#include "boundingbox.h"
#include "canvas.h"
#include "undoredo.h"
#include <QDebug>

BoundingBox::BoundingBox(BoundingBox *parent) : ConnectedToMainWindow(parent)
{
    mParent = parent;
    mParent->addChild(this);
    mTransformMatrix.reset();
    mCombinedTransformMatrix.reset();
}

BoundingBox::BoundingBox(MainWindow *window) : ConnectedToMainWindow(window)
{
    mTransformMatrix.reset();
    mCombinedTransformMatrix.reset();
}

QMatrix BoundingBox::getCombinedTransform()
{
    return mCombinedTransformMatrix;
}

QPointF BoundingBox::getTranslation()
{
    return QPointF(mTransformMatrix.dx(), mTransformMatrix.dy());
}

void BoundingBox::scale(qreal scaleBy, QPointF absOrigin)
{
    scale(scaleBy, scaleBy, absOrigin);
}

void BoundingBox::scale(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin)
{
    mTransformMatrix.translate(-absOrigin.x(), -absOrigin.y());
    mTransformMatrix.scale(scaleXBy, scaleYBy);
    mTransformMatrix.translate(absOrigin.x(), absOrigin.y());
    updateCombinedTransform();
}

void BoundingBox::rotateBy(qreal rot, QPointF absOrigin)
{
    QPointF transPoint = getAbsolutePos() - absOrigin;
    mTransformMatrix.translate(-transPoint.x(), -transPoint.y());
    mTransformMatrix.rotate(rot);
    mTransformMatrix.translate(transPoint.x(), transPoint.y());
    updateCombinedTransform();
}

void BoundingBox::moveBy(QPointF trans)
{
    moveBy(trans.x(), trans.y());
}

void BoundingBox::startTransform()
{
    mSavedTransformMatrix = mTransformMatrix;
}

void BoundingBox::finishTransform()
{
    TransformChildParentUndoRedo *undoRedo = new TransformChildParentUndoRedo(this,
                                                           mSavedTransformMatrix,
                                                           mTransformMatrix);
    addUndoRedo(undoRedo);
}

void BoundingBox::addChild(BoundingBox *child)
{
    mChildren.append(child);
}

void BoundingBox::removeChild(BoundingBox *child)
{
    mChildren.removeOne(child);
}

void BoundingBox::moveBy(qreal dx, qreal dy)
{
    mTransformMatrix.translate(dx, dy);
    updateCombinedTransform();
}

void BoundingBox::setTransformation(QMatrix transMatrix)
{
    mTransformMatrix = transMatrix;
    updateCombinedTransform();
}

QPointF BoundingBox::getAbsolutePos()
{
    return QPointF(mCombinedTransformMatrix.dx(), mCombinedTransformMatrix.dy());
}

void BoundingBox::updateAfterCombinedTransformationChanged()
{
    foreach(BoundingBox *child, mChildren) {
        child->updateCombinedTransform();
    }
}

void BoundingBox::updateCombinedTransform()
{
    if(mParent == NULL) {
        mCombinedTransformMatrix = mTransformMatrix;
    } else {
        mCombinedTransformMatrix = mTransformMatrix*
                mParent->getCombinedTransform();
    }
    updateAfterCombinedTransformationChanged();
}
