#include "pathpoint.h"
#include "vectorpath.h"
#include "undoredo.h"
#include "ctrlpoint.h"
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
    mStartCtrlPt = new CtrlPoint(startCtrlAbsPos, this, true);
    mEndCtrlPt = new CtrlPoint(endCtrlAbsPos, this, false);

    mStartCtrlPt->setOtherCtrlPt(mEndCtrlPt);
    mEndCtrlPt->setOtherCtrlPt(mStartCtrlPt);

    mStartCtrlPt->hide();
    mEndCtrlPt->hide();
}

void PathPoint::startTransform()
{
    MovablePoint::startTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->MovablePoint::startTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->MovablePoint::startTransform();
    }
}

void PathPoint::finishTransform()
{
    startNewUndoRedoSet();
    MovablePoint::finishTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->MovablePoint::finishTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->MovablePoint::finishTransform();
    }
    finishUndoRedoSet();
}

void PathPoint::moveBy(QPointF absTranslation)
{
    MovablePoint::moveBy(absTranslation);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->MovablePoint::moveBy(absTranslation);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->MovablePoint::moveBy(absTranslation);
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

qreal pointToLen(QPointF point) {
    return sqrt(point.x()*point.x() + point.y()*point.y());
}

bool isPointZero(QPointF pos) {
    return abs(pos.x()) == 0 && abs(pos.y()) == 0;
}

QPointF scalePointToNewLen(QPointF point, qreal newLen) {
    if(isPointZero(point)) {
        return point;
    }
    return point * newLen / pointToLen(point);
}

QPointF PathPoint::symmetricToAbsPosNewLen(QPointF absPosToMirror, qreal newLen)
{
    QPointF posDist = absPosToMirror - getAbsolutePos();
    return getAbsolutePos() - scalePointToNewLen(posDist, newLen);
}


void PathPoint::moveStartCtrlPtToAbsPos(QPointF startCtrlPt)
{
    mStartCtrlPt->moveToAbs(startCtrlPt);
}

QPointF PathPoint::getStartCtrlPtAbsPos()
{
    return mStartCtrlPt->getAbsolutePos();
}

QPointF PathPoint::getStartCtrlPtValue()
{
    if(mStartCtrlPtEnabled) {
        return mStartCtrlPt->getRelativePos();
    } else {
        return getRelativePos();
    }
}

MovablePoint *PathPoint::getStartCtrlPt()
{
    return mStartCtrlPt;
}

void PathPoint::ctrlPointPosChanged(bool startPtChanged) {
    ctrlPointPosChanged((startPtChanged) ? mStartCtrlPt : mEndCtrlPt,
                        (startPtChanged) ? mEndCtrlPt : mStartCtrlPt);
}

void PathPoint::ctrlPointPosChanged(CtrlPoint *pointChanged,
                                    CtrlPoint *pointToUpdate) {
    QPointF changedPointPos = pointChanged->getAbsolutePos();
    if(mCtrlsMode == CtrlsMode::CTRLS_SYMMETRIC) {
        pointToUpdate->moveToWithoutUpdatingTheOther(symmetricToAbsPos(changedPointPos));
    } else if(mCtrlsMode == CtrlsMode::CTRLS_SMOOTH) {
        if(!isPointZero(changedPointPos) ) {
            pointToUpdate->moveToWithoutUpdatingTheOther(
                        symmetricToAbsPosNewLen(
                            changedPointPos,
                            pointToLen(pointToUpdate->getAbsolutePos() -
                                       getAbsolutePos())) );
        }
    }
}

void PathPoint::moveEndCtrlPtToAbsPos(QPointF endCtrlPt)
{
    mEndCtrlPt->moveToAbs(endCtrlPt);
}

QPointF PathPoint::getEndCtrlPtAbsPos()
{
    return mEndCtrlPt->getAbsolutePos();
}

QPointF PathPoint::getEndCtrlPtValue()
{
    if(mEndCtrlPtEnabled) {
        return mEndCtrlPt->getRelativePos();
    } else {
        return getRelativePos();
    }
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
    updateEndCtrlPtVisibility();
    mVectorPath->schedulePathUpdate();
}

void PathPoint::setStartCtrlPtEnabled(bool enabled)
{
    mStartCtrlPtEnabled = enabled;
    updateStartCtrlPtVisibility();
    mVectorPath->schedulePathUpdate();
}

void PathPoint::setCtrlPtEnabled(bool enabled, bool isStartPt, bool saveUndoRedo) {
    if(isStartPt) {
        if(mStartCtrlPtEnabled == enabled) {
            return;
        }
        setStartCtrlPtEnabled(enabled);
    } else {
        if(mEndCtrlPtEnabled == enabled) {
            return;
        }
        setEndCtrlPtEnabled(enabled);
    }
    if(saveUndoRedo) {
        addUndoRedo(new SetCtrlPtEnabledUndoRedo(enabled, isStartPt, this));
    }
}

void PathPoint::setSeparatePathPoint(bool separatePathPoint)
{
    mSeparatePathPoint = separatePathPoint;
}

bool PathPoint::isSeparatePathPoint()
{
    return mSeparatePathPoint;
}

void PathPoint::setCtrlsMode(CtrlsMode mode, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        startNewUndoRedoSet();
        addUndoRedo(new SetPathPointModeUndoRedo(this, mCtrlsMode, mode));
    }
    mCtrlsMode = mode;
    if(saveUndoRedo) {
        if(mCtrlsMode == CtrlsMode::CTRLS_SYMMETRIC) {
            QPointF point1 = mEndCtrlPt->getAbsolutePos();
            point1 = symmetricToAbsPos(point1);
            QPointF point2 = mStartCtrlPt->getAbsolutePos();
            qreal len1 = pointToLen(point1);
            qreal len2 = pointToLen(point2);
            qreal lenSum = len1 + len2;
            QPointF newStartCtrlPtPos = (point1*len1 + point2*len2)/lenSum;
            QPointF newEndCtrlPtPos = symmetricToAbsPos(newStartCtrlPtPos);
            mStartCtrlPt->setAbsolutePos(newStartCtrlPtPos);
            mEndCtrlPt->setAbsolutePos(newEndCtrlPtPos);

        } else if(mCtrlsMode == CtrlsMode::CTRLS_SMOOTH) {
            QPointF point1 = mEndCtrlPt->getAbsolutePos();
            point1 = symmetricToAbsPos(point1);
            QPointF point2 = mStartCtrlPt->getAbsolutePos();
            qreal len1 = pointToLen(point1);
            qreal len2 = pointToLen(point2);
            qreal lenSum = len1 + len2;
            QPointF point1Rel = mEndCtrlPt->getAbsolutePos() - getAbsolutePos();
            QPointF point2Rel = mStartCtrlPt->getAbsolutePos() - getAbsolutePos();
            QPointF newStartDirection =
                    scalePointToNewLen(
                        (point1*len1 + point2*len2)/lenSum - getAbsolutePos(),
                        1.f);
            qreal startCtrlPtLen =
                    abs(QPointF::dotProduct(point2Rel, newStartDirection));
            QPointF newStartCtrlPtPos = newStartDirection*startCtrlPtLen +
                    getAbsolutePos();
            qreal endCtrlPtLen =
                    abs(QPointF::dotProduct(point1Rel, newStartDirection));
            QPointF newEndCtrlPtPos = -newStartDirection*endCtrlPtLen +
                    getAbsolutePos();
            mStartCtrlPt->setAbsolutePos(newStartCtrlPtPos);
            mEndCtrlPt->setAbsolutePos(newEndCtrlPtPos);
        }

        setCtrlPtEnabled(true, true);
        setCtrlPtEnabled(true, false);
        mVectorPath->schedulePathUpdate();

        finishUndoRedoSet();
    }
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
