#include "pathpoint.h"
#include "vectorpath.h"
#include "undoredo.h"
#include <QPainter>

const qreal PathPoint::RADIUS = 10.f;

PathPoint::PathPoint(QPointF absPos, VectorPath *vectorPath)
{
    mVectorPath = vectorPath;
    setAbsolutePos(absPos);
}

void PathPoint::startTransform()
{
    mSavedAbsPos = getAbsolutePos();
}

void PathPoint::finishTransform()
{
    MovePathPointUndoRedo *undoRedo = new MovePathPointUndoRedo(this,
                                                           mSavedAbsPos,
                                                           getAbsolutePos());
    getUndoRedoStack()->addUndoRedo(undoRedo);
}

UndoRedoStack *PathPoint::getUndoRedoStack() {
    return mVectorPath->getUndoRedoStack();
}

void PathPoint::connectToPoint(PathPoint *point)
{
    if(point == NULL) {
        return;
    }
    if(!hasNextPoint()) {
        setPointAsNext(point);
    } else if(!hasPreviousPoint()) {
        setPointAsPrevious(point);
    } else {
        return;
    }
}

void PathPoint::disconnectFromPoint(PathPoint *point)
{
    if(point == NULL) {
        return;
    }
    if(point == mNextPoint) {
        setPointAsNext(NULL);
    } else if(point == mPreviousPoint) {
        setPointAsPrevious(NULL);
    } else {
        return;
    }
}

void PathPoint::remove()
{
    mVectorPath->removePoint(this);
}

void PathPoint::setAbsolutePos(QPointF pos)
{
    setRelativePos(mVectorPath->getCombinedTransform().inverted().map(pos));
}

void PathPoint::setRelativePos(QPointF pos)
{
    mRelativePos.setCurrentValue(pos, false);
    mVectorPath->schedulePathUpdate();
}

QPointF PathPoint::getRelativePos()
{
    return mRelativePos.getCurrentValue();
}

QPointF PathPoint::getAbsolutePos()
{
    return mVectorPath->getCombinedTransform().map(getRelativePos());
}

void PathPoint::setStartCtrlPt(QPointF startCtrlPt)
{
    mStartCtrlPt.setCurrentValue(startCtrlPt, false);
    mVectorPath->schedulePathUpdate();
}

QPointF PathPoint::getStartCtrlPt()
{
    return mStartCtrlPt.getCurrentValue();
}

void PathPoint::setEndCtrlPt(QPointF endCtrlPt)
{
    mEndCtrlPt.setCurrentValue(endCtrlPt, false);
    mVectorPath->schedulePathUpdate();
}

QPointF PathPoint::getEndCtrlPt()
{
    return mEndCtrlPt.getCurrentValue();
}

void PathPoint::draw(QPainter *p)
{
    if(mSelected) {
        p->setBrush(QColor(0, 0, 255, 75));
    } else {
        p->setBrush(Qt::NoBrush);
    }
    p->drawEllipse(getAbsolutePos(),
                   RADIUS, RADIUS);
}

PathPoint* PathPoint::getNextPoint()
{
    return mNextPoint;
}

PathPoint *PathPoint::getPreviousPoint()
{
    return mPreviousPoint;
}

void PathPoint::setNextPoint(PathPoint *nextPoint, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        SetNextPointUndoRedo *undoRedo = new SetNextPointUndoRedo(this,
                                                                  mNextPoint,
                                                                  nextPoint);
        getUndoRedoStack()->addUndoRedo(undoRedo);
    }
    mNextPoint = nextPoint;
    mVectorPath->schedulePathUpdate();
}

void PathPoint::setPreviousPoint(PathPoint *previousPoint, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        SetPreviousPointUndoRedo *undoRedo = new SetPreviousPointUndoRedo(this,
                                                                      mPreviousPoint,
                                                                      previousPoint);
        getUndoRedoStack()->addUndoRedo(undoRedo);
    }
    mPreviousPoint = previousPoint;
    mVectorPath->schedulePathUpdate();
}

bool PathPoint::hasNextPoint() {
    return mNextPoint != NULL;
}

bool PathPoint::hasPreviousPoint() {
    return mPreviousPoint != NULL;
}

void PathPoint::setPointAsNext(PathPoint *pointToSet) {
    UndoRedoStack *stack = getUndoRedoStack();
    stack->startNewSet();

    if(hasNextPoint()) {
        mNextPoint->setPreviousPoint(NULL);
    }
    setNextPoint(pointToSet);
    if(pointToSet != NULL) {
        pointToSet->setPreviousPoint(this);
    }

    stack->finishSet();
}

void PathPoint::setPointAsPrevious(PathPoint *pointToSet) {
    UndoRedoStack *stack = getUndoRedoStack();
    stack->startNewSet();

    if(hasPreviousPoint()) {
        mPreviousPoint->setNextPoint(NULL);
    }
    setPreviousPoint(pointToSet);
    if(pointToSet != NULL) {
        pointToSet->setNextPoint(this);
    }

    stack->finishSet();
}

VectorPath *PathPoint::getParentPath()
{
    return mVectorPath;
}

PathPoint *PathPoint::addPoint(QPointF absPos)
{
    return mVectorPath->addPoint(absPos, this);
}

bool PathPoint::isEndPoint() {
    return mNextPoint == NULL || mPreviousPoint == NULL;
}

bool PathPoint::isPointAt(QPointF absPoint)
{
    QPointF dist = getAbsolutePos() - absPoint;
    return (dist.x()*dist.x() + dist.y()*dist.y() < RADIUS*RADIUS);
}

bool PathPoint::isContainedInRect(QRectF absRect)
{
    return absRect.contains(getAbsolutePos());
}

void PathPoint::moveBy(QPointF absTranslation) {
    setAbsolutePos(getAbsolutePos() + absTranslation);
}

void PathPoint::select()
{
    mSelected = true;
}

void PathPoint::deselect()
{
    mSelected = false;
}

bool PathPoint::isSelected()
{
    return mSelected;
}
