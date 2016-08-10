#include "movablepoint.h"
#include "vectorpath.h"
#include "undoredo.h"

MovablePoint::MovablePoint(QPointF absPos,
                           BoundingBox *parent,
                           MovablePointType type,
                           qreal radius) : ConnectedToMainWindow(parent)
{
    mType = type;
    mRadius = radius;
    mParent = parent;
    setAbsolutePos(absPos, false);
}

void MovablePoint::startTransform()
{
    mTransformStarted = true;
    mSavedRelPos = getRelativePos();
}

void MovablePoint::finishTransform()
{
    if(!mTransformStarted) {
        return;
    }
    mTransformStarted = false;
    MoveMovablePointUndoRedo *undoRedo = new MoveMovablePointUndoRedo(this,
                                                           mSavedRelPos,
                                                           getRelativePos());
    addUndoRedo(undoRedo);
}

void MovablePoint::setAbsolutePos(QPointF pos, bool saveUndoRedo)
{
    setRelativePos(mParent->getCombinedTransform().inverted().map(pos), saveUndoRedo );
}

void MovablePoint::setRelativePos(QPointF relPos, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        addUndoRedo(new MoveMovablePointUndoRedo(this, getRelativePos(), relPos));
    }
    mRelPos.setCurrentValue(relPos, false);
}

QPointF MovablePoint::getRelativePos()
{
    return mRelPos.getCurrentValue();
}

QPointF MovablePoint::getAbsolutePos()
{
    return mParent->getCombinedTransform().map(getRelativePos());
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

BoundingBox *MovablePoint::getParent()
{
    return mParent;
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
    mParent->scheduleRepaint();
}

void MovablePoint::deselect()
{
    mSelected = false;
    mParent->scheduleRepaint();
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
