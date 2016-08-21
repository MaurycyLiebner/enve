#include "movablepoint.h"
#include "vectorpath.h"
#include "undoredo.h"

MovablePoint::MovablePoint(QPointF absPos,
                           BoundingBox *parent,
                           MovablePointType type,
                           qreal radius) : Transformable(parent)
{
    mType = type;
    mRadius = radius;
    mParent = parent;
    setAbsolutePos(absPos, false);
}

MovablePoint::MovablePoint(qreal relPosX, qreal relPosY,
                           BoundingBox *parent,
                           MovablePointType type,
                           qreal radius) : Transformable(parent)
{
    mType = type;
    mRadius = radius;
    mParent = parent;
    setRelativePos(QPointF(relPosX, relPosY), false);
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

void MovablePoint::scale(qreal scaleBy) {
    scale(scaleBy, scaleBy);
}

void MovablePoint::cancelTransform()
{
    setRelativePos(mSavedRelPos, false);
}

void MovablePoint::setRadius(qreal radius)
{
    mRadius = radius;
}

QPointF MovablePoint::getAbsBoneAttachPoint()
{
    return getAbsolutePos();
}

void MovablePoint::attachToBoneFromSqlZId()
{
    setBone(mParent->getParent()->boneFromZIndex(mSqlLoadBoneZId) );
}

bool MovablePoint::isBeingTransformed()
{
    return mSelected || mParent->isSelected();
}

void MovablePoint::rotateBy(qreal rot)
{
    QMatrix rotMatrix;
    rotMatrix.translate(-mSavedTransformPivot.x(), -mSavedTransformPivot.y());
    rotMatrix.rotate(rot);
    rotMatrix.translate(mSavedTransformPivot.x(), mSavedTransformPivot.y());
    setRelativePos(mSavedRelPos*rotMatrix, false);
}

void MovablePoint::scale(qreal scaleXBy, qreal scaleYBy)
{
    QMatrix scaleMatrix;
    scaleMatrix.translate(-mSavedTransformPivot.x(), -mSavedTransformPivot.y());
    scaleMatrix.scale(scaleXBy, scaleYBy);
    scaleMatrix.translate(mSavedTransformPivot.x(), mSavedTransformPivot.y());
    setRelativePos(mSavedRelPos*scaleMatrix, false);
}

void MovablePoint::saveTransformPivot(QPointF absPivot)
{
    mSavedTransformPivot =
            -mParent->getCombinedTransform().inverted().map(absPivot);
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
