#include "movablepoint.h"
#include "vectorpath.h"
#include "undoredo.h"

MovablePoint::MovablePoint(QPointF absPos, VectorPath *vectorPath, qreal radius) : ConnectedToMainWindow(vectorPath)
{
    mRadius = radius;
    mVectorPath = vectorPath;
    setAbsolutePos(absPos);
}

void MovablePoint::startTransform()
{
    mSavedAbsPos = getAbsolutePos();
}

void MovablePoint::finishTransform()
{
    MoveMovablePointUndoRedo *undoRedo = new MoveMovablePointUndoRedo(this,
                                                           mSavedAbsPos,
                                                           getAbsolutePos());
    addUndoRedo(undoRedo);
}

void MovablePoint::setAbsolutePos(QPointF pos)
{
    setRelativePos(mVectorPath->getCombinedTransform().inverted().map(pos));
}

void MovablePoint::setRelativePos(QPointF pos)
{
    mRelativePos.setCurrentValue(pos, false);
    mVectorPath->schedulePathUpdate();
}

QPointF MovablePoint::getRelativePos()
{
    return mRelativePos.getCurrentValue();
}

QPointF MovablePoint::getAbsolutePos()
{
    return mVectorPath->getCombinedTransform().map(getRelativePos());
}

void MovablePoint::draw(QPainter *p)
{
    if(mSelected) {
        p->setBrush(QColor(255, 0, 0, 200));
    } else {
        p->setBrush(QColor(255, 0, 0, 125));
    }
    p->drawEllipse(getAbsolutePos(),
                   mRadius, mRadius);
}

VectorPath *MovablePoint::getParentPath()
{
    return mVectorPath;
}

bool MovablePoint::isPointAt(QPointF absPoint)
{
    QPointF dist = getAbsolutePos() - absPoint;
    return (dist.x()*dist.x() + dist.y()*dist.y() < mRadius*mRadius);
}

bool MovablePoint::isContainedInRect(QRectF absRect)
{
    return absRect.contains(getAbsolutePos());
}

void MovablePoint::moveBy(QPointF absTranslation) {
    setAbsolutePos(getAbsolutePos() + absTranslation);
}

void MovablePoint::select()
{
    mSelected = true;
    mVectorPath->scheduleRepaint();
}

void MovablePoint::deselect()
{
    mSelected = false;
    mVectorPath->scheduleRepaint();
}

bool MovablePoint::isSelected()
{
    return mSelected;
}

void MovablePoint::remove()
{

}
