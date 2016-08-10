#include "pathpoint.h"
#include "vectorpath.h"
#include "undoredo.h"
#include <QPainter>

PathPoint::PathPoint(QPointF absPos, VectorPath *vectorPath) :
    PathPoint(absPos, absPos, absPos, vectorPath)
{

}

PathPoint::PathPoint(QPointF absPos,
                     QPointF startCtrlAbsPos,
                     QPointF endCtrlAbsPos,
                     VectorPath *vectorPath) :
    MovablePoint(absPos, vectorPath, MovablePointType::TYPE_PATH_POINT, 10.f)
{
    mStartCtrlPt = new MovablePoint(startCtrlAbsPos, vectorPath, MovablePointType::TYPE_CTRL_POINT);
    mStartCtrlPt->hide();
    mEndCtrlPt = new MovablePoint(endCtrlAbsPos, vectorPath, MovablePointType::TYPE_CTRL_POINT);
    mEndCtrlPt->hide();
}

void PathPoint::startTransform()
{
    MovablePoint::startTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->startTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->startTransform();
    }
}

void PathPoint::finishTransform()
{
    startNewUndoRedoSet();
    MovablePoint::finishTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->finishTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->finishTransform();
    }
    finishUndoRedoSet();
}

void PathPoint::moveBy(QPointF absTranslation)
{
    MovablePoint::moveBy(absTranslation);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->moveBy(absTranslation);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->moveBy(absTranslation);
    }
}

void PathPoint::moveToAbs(QPointF absPos)
{
    moveBy(absPos - getAbsolutePos());
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

void PathPoint::rectPointsSelection(QRectF absRect, QList<MovablePoint*> *list) {
    if(!isSelected()) {
        if(isContainedInRect(absRect)) {
            select();
            list->append(this);
        }
    }
    if(!mEndCtrlPt->isSelected()) {
        if(mEndCtrlPt->isContainedInRect(absRect)) {
            mEndCtrlPt->select();
            list->append(mEndCtrlPt);
        }
    }
    if(!mStartCtrlPt->isSelected()) {
        if(mStartCtrlPt->isContainedInRect(absRect)) {
            mStartCtrlPt->select();
            list->append(mStartCtrlPt);
        }
    }
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

QPointF PathPoint::symmetricToAbsPos(QPointF absPosToMirror) {
    QPointF posDist = absPosToMirror - getAbsolutePos();
    return getAbsolutePos() - posDist;
}

void PathPoint::setStartCtrlPtAbsPos(QPointF startCtrlPt)
{
    if(mCtrlsMode == CtrlsMode::CTRLS_SYMMETRIC) {
        mEndCtrlPt->setAbsolutePos(symmetricToAbsPos(startCtrlPt));
    }
    mStartCtrlPt->setAbsolutePos(startCtrlPt);
}

QPointF PathPoint::getStartCtrlPtAbsPos()
{
    return mStartCtrlPt->getAbsolutePos();
}

QPointF PathPoint::getStartCtrlPtRelativePos()
{
    return mStartCtrlPt->getRelativePos();
}

MovablePoint *PathPoint::getStartCtrlPt()
{
    return mStartCtrlPt;
}

void PathPoint::setEndCtrlPtAbsPos(QPointF endCtrlPt)
{
    if(mCtrlsMode == CtrlsMode::CTRLS_SYMMETRIC) {
        mStartCtrlPt->setAbsolutePos(symmetricToAbsPos(endCtrlPt));
    }
    mEndCtrlPt->setAbsolutePos(endCtrlPt);
}

QPointF PathPoint::getEndCtrlPtAbsPos()
{
    return mEndCtrlPt->getAbsolutePos();
}

QPointF PathPoint::getEndCtrlPtRelativePos()
{
    return mEndCtrlPt->getRelativePos();
}

MovablePoint *PathPoint::getEndCtrlPt()
{
    return mEndCtrlPt;
}

void PathPoint::draw(QPainter *p, CanvasMode mode)
{
    if(mSelected) {
        p->setBrush(QColor(0, 0, 255, 155));
    } else {
        p->setBrush(QColor(0, 0, 255, 75));
    }
    p->drawEllipse(getAbsolutePos(),
                   mRadius, mRadius);
    if(mode == CanvasMode::MOVE_POINT || (mode == CanvasMode::ADD_POINT && mSelected)) {
        QPen pen = p->pen();
        p->setPen(QPen(Qt::black, 1.5f, Qt::DotLine));
        if(mEndCtrlPt->isVisible() || mode == CanvasMode::ADD_POINT) {
            p->drawLine(getAbsolutePos(), mEndCtrlPt->getAbsolutePos());
        }
        if(mStartCtrlPt->isVisible() || mode == CanvasMode::ADD_POINT) {
            p->drawLine(getAbsolutePos(), mStartCtrlPt->getAbsolutePos());
        }
        p->setPen(pen);
        mEndCtrlPt->draw(p);
        mStartCtrlPt->draw(p);
    }
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
    updateEndCtrlPtVisibility();
    mVectorPath->schedulePathUpdate();
}

void PathPoint::updateStartCtrlPtVisibility() {
    if(mPreviousPoint == NULL) {
        mStartCtrlPt->hide();
    } else {
        mStartCtrlPt->setVisible(mStartCtrlPtEnabled);
    }
}

void PathPoint::updateEndCtrlPtVisibility() {
    if(mNextPoint == NULL) {
        mEndCtrlPt->hide();
    } else {
        mEndCtrlPt->setVisible(mEndCtrlPtEnabled);
    }
}

void PathPoint::setEndCtrlPtEnabled(bool enabled)
{
    mEndCtrlPtEnabled = enabled;
    mEndCtrlPt->setAbsolutePos(getAbsolutePos());
    updateEndCtrlPtVisibility();
}

void PathPoint::setStartCtrlPtEnabled(bool enabled)
{
    mStartCtrlPtEnabled = enabled;
    mStartCtrlPt->setAbsolutePos(getAbsolutePos());
    updateStartCtrlPtVisibility();
}

void PathPoint::setSeparatePathPoint(bool separatePathPoint)
{
    mSeparatePathPoint = separatePathPoint;
}

bool PathPoint::isSeparatePathPoint()
{
    return mSeparatePathPoint;
}

void PathPoint::setCtrlsMode(CtrlsMode mode)
{
    mCtrlsMode = mode;
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
    updateStartCtrlPtVisibility();
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

PathPoint *PathPoint::addPoint(PathPoint *pointToAdd)
{
    return mVectorPath->addPoint(pointToAdd, this);
}

bool PathPoint::isEndPoint() {
    return mNextPoint == NULL || mPreviousPoint == NULL;
}
