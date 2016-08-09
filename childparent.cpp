#include "childparent.h"
#include "canvas.h"
#include "undoredo.h"
#include <QDebug>

ChildParent::ChildParent(Canvas *canvas)
{
    mCanvas = canvas;
    mTransformMatrix.reset();
    mCombinedTransformMatrix.reset();
}

QMatrix ChildParent::getCombinedTransform()
{
    return mCombinedTransformMatrix;
}

QPointF ChildParent::getTranslation()
{
    return QPointF(mTransformMatrix.dx(), mTransformMatrix.dy());
}

void ChildParent::rotateBy(qreal rot, QPointF absOrigin)
{
    mRot += rot;
    QPointF transPoint = getAbsolutePos() - absOrigin;
    mTransformMatrix.translate(-transPoint.x(), -transPoint.y());
    mTransformMatrix.rotate(rot);
    mTransformMatrix.translate(transPoint.x(), transPoint.y());
    updateCombinedTransform();
    updateAfterTransformationChanged();
}

void ChildParent::moveBy(QPointF trans)
{
    moveBy(trans.x(), trans.y());
}

void ChildParent::startTransform()
{
    mSavedTransformMatrix = mTransformMatrix;
}

void ChildParent::finishTransform()
{
    TransformChildParentUndoRedo *undoRedo = new TransformChildParentUndoRedo(this,
                                                           mSavedTransformMatrix,
                                                           mTransformMatrix);
    getUndoRedoStack()->addUndoRedo(undoRedo);
}

UndoRedoStack *ChildParent::getUndoRedoStack()
{
    return mCanvas->getUndoRedoStack();
}

void ChildParent::moveBy(qreal dx, qreal dy)
{
    mTransformMatrix.translate(dx, dy);
    updateCombinedTransform();
    updateAfterTransformationChanged();
}

void ChildParent::setTransformation(QMatrix transMatrix)
{
    mTransformMatrix = transMatrix;
    updateCombinedTransform();
    updateAfterTransformationChanged();
}

QPointF ChildParent::getAbsolutePos()
{
    return QPointF(mCombinedTransformMatrix.dx(), mCombinedTransformMatrix.dy());
}

void ChildParent::updateAfterTransformationChanged()
{
    qDebug() << "ChildParent::updateAfterTransformationChanged CALLED!";
}

void ChildParent::updateAfterCombinedTransformationChanged()
{
    qDebug() << "ChildParent::updateAfterCombinedTransformationChanged CALLED!";
}

void ChildParent::updateCombinedTransform()
{
    if(mParent == NULL) {
        mCombinedTransformMatrix = mTransformMatrix;
    } else {
        mCombinedTransformMatrix = mParent->getCombinedTransform()*mTransformMatrix;
    }
    updateAfterCombinedTransformationChanged();
}
