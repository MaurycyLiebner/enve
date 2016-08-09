#include "pathpoint.h"
#include "vectorpath.h"
#include "undoredo.h"
#include <QPainter>

PathPoint::PathPoint(QPointF absPos, VectorPath *vectorPath) : MovablePoint(absPos, vectorPath, 10.f)
{
    mStartCtrlPt = new MovablePoint(absPos, vectorPath);
    mEndCtrlPt = new MovablePoint(absPos, vectorPath);
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

void PathPoint::moveBy(QPointF absTranslation)
{
    MovablePoint::moveBy(absTranslation);
    mStartCtrlPt->moveBy(absTranslation);
    mEndCtrlPt->moveBy(absTranslation);
}

MovablePoint *PathPoint::getPointAtAbsPos(QPointF absPos, CanvasMode canvasMode)
{
    if(canvasMode == CanvasMode::MOVE_POINT) {
        if(mStartCtrlPt->isPointAt(absPos)) {
            return mStartCtrlPt;
        } else if (mEndCtrlPt->isPointAt(absPos)) {
            return mEndCtrlPt;
        }
    }
    if (isPointAt(absPos)) {
        return this;
    }
    return NULL;
}

void PathPoint::setStartCtrlPtRelativePos(QPointF startCtrlPt)
{
    mStartCtrlPt->setRelativePos(startCtrlPt);
}

QPointF PathPoint::getStartCtrlPtRelativePos()
{
    return mStartCtrlPt->getRelativePos();
}

MovablePoint *PathPoint::getStartCtrlPt()
{
    return mStartCtrlPt;
}

void PathPoint::setEndCtrlPtRelativePos(QPointF endCtrlPt)
{
    mEndCtrlPt->setRelativePos(endCtrlPt);
}

QPointF PathPoint::getEndCtrlPtRelativePos()
{
    return mEndCtrlPt->getRelativePos();
}

MovablePoint *PathPoint::getEndCtrlPt()
{
    return mEndCtrlPt;
}

void PathPoint::draw(QPainter *p)
{
    if(mSelected) {
        p->setBrush(QColor(0, 0, 255, 75));
    } else {
        p->setBrush(Qt::NoBrush);
    }
    p->drawEllipse(getAbsolutePos(),
                   mRadius, mRadius);
    mEndCtrlPt->draw(p);
    mStartCtrlPt->draw(p);
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
        addUndoRedo(undoRedo);
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
        addUndoRedo(undoRedo);
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
    startNewUndoRedoSet();

    if(hasNextPoint()) {
        mNextPoint->setPreviousPoint(NULL);
    }
    setNextPoint(pointToSet);
    if(pointToSet != NULL) {
        pointToSet->setPreviousPoint(this);
    }

    finishUndoRedoSet();
}

void PathPoint::setPointAsPrevious(PathPoint *pointToSet) {
    startNewUndoRedoSet();

    if(hasPreviousPoint()) {
        mPreviousPoint->setNextPoint(NULL);
    }
    setPreviousPoint(pointToSet);
    if(pointToSet != NULL) {
        pointToSet->setNextPoint(this);
    }

    finishUndoRedoSet();
}

PathPoint *PathPoint::addPoint(QPointF absPos)
{
    return mVectorPath->addPoint(absPos, this);
}

bool PathPoint::isEndPoint() {
    return mNextPoint == NULL || mPreviousPoint == NULL;
}
