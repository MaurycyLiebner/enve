#include "movablepoint.h"
#include "vectorpath.h"
#include "undoredo.h"

MovablePoint::MovablePoint(QPointF absPos, VectorPath *vectorPath, MovablePointType type, qreal radius) : ConnectedToMainWindow(vectorPath)
{
    mType = type;
    mRadius = radius;
    mVectorPath = vectorPath;
    setAbsolutePos(absPos, false);
}

void MovablePoint::startTransform()
{
    mTransformStarted = true;
    mSavedAbsPos = getAbsolutePos();
}

void MovablePoint::finishTransform()
{
    if(!mTransformStarted) {
        return;
    }
    mTransformStarted = false;
    MoveMovablePointUndoRedo *undoRedo = new MoveMovablePointUndoRedo(this,
                                                           mSavedAbsPos,
                                                           getAbsolutePos());
    addUndoRedo(undoRedo);
}

void MovablePoint::setAbsolutePos(QPointF pos, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        addUndoRedo(new MoveMovablePointUndoRedo(this, getAbsolutePos(), pos));
    }
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
        p->setBrush(QColor(255, 0, 0, 155));
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
    setAbsolutePos(getAbsolutePos() + absTranslation, false);
}

void MovablePoint::moveToAbs(QPointF absPos)
{
    setAbsolutePos(absPos, false);
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

bool MovablePoint::isPathPoint()
{
    return mType == MovablePointType::TYPE_PATH_POINT;
}

bool MovablePoint::isPivotPoint()
{
    return mType == MovablePointType::TYPE_PIVOT_POINT;
}

bool MovablePoint::isCtrlPoint()
{
    return mType == MovablePointType::TYPE_CTRL_POINT;
}
