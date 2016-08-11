#include "boundingbox.h"
#include "canvas.h"
#include "undoredo.h"
#include <QDebug>

BoundingBox::BoundingBox(BoundingBox *parent, BoundingBoxType type) : ConnectedToMainWindow(parent)
{
    mType = type;
    mParent = parent;
    mParent->addChild(this);
    mTransformMatrix.reset();
    mCombinedTransformMatrix.reset();
    updateCombinedTransform();
}

QPointF BoundingBox::getPivotAbsPos()
{
    return mAbsRotPivotPos;
}

bool BoundingBox::isSelected()
{
    return mSelected;
}

void BoundingBox::select()
{
    mSelected = true;
    scheduleRepaint();
}

void BoundingBox::deselect()
{
    mSelected = false;
    scheduleRepaint();
}

BoundingBox::BoundingBox(MainWindow *window, BoundingBoxType type) : ConnectedToMainWindow(window)
{
    mType = type;
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
    startNewUndoRedoSet();
    addChildToListAt(0, child);
    updateChildrenId(0);
    finishUndoRedoSet();
}

void BoundingBox::addChildToListAt(int index, BoundingBox *child, bool saveUndoRedo) {
    mChildren.insert(index, child);

    if(saveUndoRedo) {
        addUndoRedo(new AddChildToListUndoRedo(this, index, child));
    }
}

void BoundingBox::updateChildrenId(int firstId) {
    updateChildrenId(firstId, mChildren.length() - 1);
}

void BoundingBox::updateChildrenId(int firstId, int lastId) {
    startNewUndoRedoSet();
    for(int i = firstId; i <= lastId; i++) {
        mChildren.at(i)->setZListIndex(i);
    }
    finishUndoRedoSet();
}

void BoundingBox::removeChildFromList(int id, bool saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new RemoveChildFromListUndoRedo(this, id, mChildren.at(id)) );
    }
    mChildren.removeAt(id);
}

void BoundingBox::removeChild(BoundingBox *child)
{
    startNewUndoRedoSet();
    int index = mChildren.indexOf(child);
    removeChildFromList(index);
    updateChildrenId(index);
    finishUndoRedoSet();
}

void BoundingBox::moveUp()
{
    mParent->decreaseChildZInList(this);
}

void BoundingBox::moveDown()
{
    mParent->increaseChildZInList(this);
}

void BoundingBox::bringToFront()
{
    mParent->bringChildToFrontList(this);
}

void BoundingBox::bringToEnd()
{
    mParent->bringChildToEndList(this);
}

void BoundingBox::increaseChildZInList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == mChildren.count() - 1) {
        return;
    }
    startNewUndoRedoSet();
    moveChildInList(index, index + 1);
    updateChildrenId(index);
    finishUndoRedoSet();
}

void BoundingBox::decreaseChildZInList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == 0) {
        return;
    }
    startNewUndoRedoSet();
    moveChildInList(index, index - 1);
    updateChildrenId(index - 1);
    finishUndoRedoSet();
}

void BoundingBox::bringChildToEndList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == mChildren.count() - 1) {
        return;
    }
    startNewUndoRedoSet();
    moveChildInList(index, mChildren.length() - 1);
    updateChildrenId(index - 1);
    finishUndoRedoSet();
}

void BoundingBox::bringChildToFrontList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == 0) {
        return;
    }
    startNewUndoRedoSet();
    moveChildInList(index, 0);
    updateChildrenId(0, index);
    finishUndoRedoSet();
}

void BoundingBox::moveChildInList(int from, int to, bool saveUndoRedo) {
    mChildren.move(from, to);
    if(saveUndoRedo) {
        addUndoRedo(new MoveChildInListUndoRedo(from, to, this) );
    }
}

void BoundingBox::setZListIndex(int z, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        addUndoRedo(new SetBoundingBoxZListIndexUnoRedo(mZListIndex, z, this));
    }
    mZListIndex = z;
}

int BoundingBox::getZIndex() {
    return mZListIndex;
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
