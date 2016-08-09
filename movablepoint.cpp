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
    if(mHidden) {
        return;
    }
    if(mSelected) {
        p->setBrush(QColor(255, 0, 0, 255));
    } else {
        p->setBrush(QColor(255, 0, 0, 75));
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
    if(mHidden) {
        return false;
    }
    QPointF dist = getAbsolutePos() - absPoint;
    return (dist.x()*dist.x() + dist.y()*dist.y() < mRadius*mRadius);
}

bool MovablePoint::isContainedInRect(QRectF absRect)
{
    if(mHidden) {
        return false;
    }
    return absRect.contains(getAbsolutePos());
}

void MovablePoint::moveBy(QPointF absTranslation) {
    setAbsolutePos(getAbsolutePos() + absTranslation);
}

void MovablePoint::moveToAbs(QPointF absPos)
{
    setAbsolutePos(absPos);
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

void MovablePoint::hide()
{
    mHidden = true;
    deselect();
}

void MovablePoint::show()
{
    mHidden = false;
}

bool MovablePoint::isHidden()
{
    return mHidden;
}

bool MovablePoint::isVisible()
{
    return !mHidden;
}

void MovablePoint::setVisible(bool bT)
{
    if(bT) {
        show();
    } else {
        hide();
    }
}
