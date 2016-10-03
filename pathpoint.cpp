#include "pathpoint.h"
#include "vectorpath.h"
#include "undoredo.h"
#include "ctrlpoint.h"
#include <QPainter>
#include <QDebug>

PathPoint::PathPoint(QPointF absPos, VectorPath *vectorPath) :
    PathPoint(absPos, absPos, absPos, vectorPath)
{

}

PathPoint::PathPoint(qreal relPosX, qreal relPosy,
                     qreal startCtrlRelX, qreal startCtrlRelY,
                     qreal endCtrlRelX, qreal endCtrlRelY, bool isFirst, int boneZ,
                     VectorPath *vectorPath) :
    MovablePoint(relPosX, relPosy, vectorPath, MovablePointType::TYPE_PATH_POINT, 10.f)
{    
    mSqlLoadBoneZId = boneZ;
    mSeparatePathPoint = isFirst;
    mVectorPath = vectorPath;
    mStartCtrlPt = new CtrlPoint(startCtrlRelX, startCtrlRelY, this, true);
    mEndCtrlPt = new CtrlPoint(endCtrlRelX, endCtrlRelY, this, false);
    mStartCtrlPt->incNumberPointers();
    mEndCtrlPt->incNumberPointers();

    mStartCtrlPt->setOtherCtrlPt(mEndCtrlPt);
    mEndCtrlPt->setOtherCtrlPt(mStartCtrlPt);

    mStartCtrlPt->hide();
    mEndCtrlPt->hide();

    setPosAnimatorUpdater(new PathPointUpdater(vectorPath) );

    mPathPointAnimators.setAllVars(this,
                                   mEndCtrlPt->getRelativePosAnimatorPtr(),
                                   mStartCtrlPt->getRelativePosAnimatorPtr(),
                                   getRelativePosAnimatorPtr());

    mPathPointAnimators.incNumberPointers();

    mRelPos.setTraceKeyOnCurrentFrame(true);
}

void PathPoint::clearAll()
{
    mStartCtrlPt->decNumberPointers();
    mEndCtrlPt->decNumberPointers();
}

PathPoint::PathPoint(QPointF absPos,
                     QPointF startCtrlAbsPos,
                     QPointF endCtrlAbsPos,
                     VectorPath *vectorPath) :
    MovablePoint(absPos, vectorPath, MovablePointType::TYPE_PATH_POINT, 10.f)
{
    mVectorPath = vectorPath;

    mStartCtrlPt = new CtrlPoint(startCtrlAbsPos, this, true);
    mEndCtrlPt = new CtrlPoint(endCtrlAbsPos, this, false);
    mStartCtrlPt->incNumberPointers();
    mEndCtrlPt->incNumberPointers();

    mStartCtrlPt->setOtherCtrlPt(mEndCtrlPt);
    mEndCtrlPt->setOtherCtrlPt(mStartCtrlPt);

    mStartCtrlPt->hide();
    mEndCtrlPt->hide();

    setPosAnimatorUpdater(new PathPointUpdater(vectorPath) );

    mPathPointAnimators.setAllVars(this,
                                   mEndCtrlPt->getRelativePosAnimatorPtr(),
                                   mStartCtrlPt->getRelativePosAnimatorPtr(),
                                   getRelativePosAnimatorPtr());
    mPathPointAnimators.incNumberPointers();

    mRelPos.setTraceKeyOnCurrentFrame(true);
}

PathPoint::PathPoint(int movablePointId, int pathPointId,
                     VectorPath *vectorPath) :
    MovablePoint(movablePointId, vectorPath,
                 MovablePointType::TYPE_PATH_POINT, 10.f)
{
    mVectorPath = vectorPath;

    QSqlQuery query;
    QString queryStr = "SELECT * FROM pathpoint WHERE id = " +
            QString::number(pathPointId);
    if(query.exec(queryStr)) {
        query.next();
        int idisfirst = query.record().indexOf("isfirst");
        int idstartctrlptid = query.record().indexOf("startctrlptid");
        int idendctrlptid = query.record().indexOf("endctrlptid");

        mSeparatePathPoint = query.value(idisfirst).toBool();

        mStartCtrlPt = new CtrlPoint(query.value(idstartctrlptid).toInt(),
                                     this, true);
        mEndCtrlPt = new CtrlPoint(query.value(idendctrlptid).toInt(),
                                   this, false);
        mStartCtrlPt->incNumberPointers();
        mEndCtrlPt->incNumberPointers();
        mStartCtrlPt->setOtherCtrlPt(mEndCtrlPt);
        mEndCtrlPt->setOtherCtrlPt(mStartCtrlPt);
    } else {
        qDebug() << "Could not load pathpoint with id " << pathPointId;
    }

    mPathPointAnimators.setAllVars(this,
                                   mEndCtrlPt->getRelativePosAnimatorPtr(),
                                   mStartCtrlPt->getRelativePosAnimatorPtr(),
                                   getRelativePosAnimatorPtr());
    mPathPointAnimators.incNumberPointers();

    mRelPos.setTraceKeyOnCurrentFrame(true);
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

void PathPoint::cancelTransform()
{
    MovablePoint::cancelTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->MovablePoint::cancelTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->MovablePoint::cancelTransform();
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

#include <QSqlError>
void PathPoint::saveToSql(int vectorPathId)
{
    int movablePtId = MovablePoint::saveToSql();
    int startPtId = mStartCtrlPt->saveToSql();
    int endPtId = mEndCtrlPt->saveToSql();
    QSqlQuery query;
    QString isFirst = ( (mSeparatePathPoint) ? "1" : "0" );
    QString isEnd = ( (isEndPoint()) ? "1" : "0" );
    if(!query.exec(QString("INSERT INTO pathpoint (isfirst, isendpoint, "
                "movablepointid, startctrlptid, endctrlptid, vectorpathid) "
                "VALUES (%1, %2, %3, %4, %5, %6)").
                arg(isFirst).
                arg(isEnd).
                arg(movablePtId).
                arg(startPtId).
                arg(endPtId).
                arg(vectorPathId) ) ) {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }
    if(mNextPoint != NULL) {
        if(!mNextPoint->isSeparatePathPoint()) {
            mNextPoint->saveToSql( vectorPathId);
        }
    }
}

void PathPoint::attachToBoneFromSqlZId()
{
    MovablePoint::attachToBoneFromSqlZId();
    mStartCtrlPt->attachToBoneFromSqlZId();
    mEndCtrlPt->attachToBoneFromSqlZId();
}

QPointF PathPoint::symmetricToAbsPos(QPointF absPosToMirror) {
    return symmetricToPos(absPosToMirror, getAbsolutePos());
}

QPointF PathPoint::symmetricToAbsPosNewLen(QPointF absPosToMirror, qreal newLen)
{
    return symmetricToPosNewLen(absPosToMirror, getAbsolutePos(), newLen);
}

void PathPoint::moveStartCtrlPtToAbsPos(QPointF startCtrlPt)
{
    mStartCtrlPt->moveToAbs(startCtrlPt);
}

void PathPoint::moveEndCtrlPtToRelPos(QPointF endCtrlPt)
{
    mEndCtrlPt->setRelativePos(endCtrlPt);
}

void PathPoint::moveStartCtrlPtToRelPos(QPointF startCtrlPt)
{
    mStartCtrlPt->setRelativePos(startCtrlPt);
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

CtrlPoint *PathPoint::getStartCtrlPt()
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

CtrlPoint *PathPoint::getEndCtrlPt()
{
    return mEndCtrlPt;
}

void PathPoint::draw(QPainter *p, CanvasMode mode)
{
    if(mSelected) {
        p->setBrush(QColor(0, 200, 255));
    } else {
        p->setBrush(QColor(170, 240, 255));
    }
    QPointF absPos = getAbsolutePos();
    p->drawEllipse(absPos,
                   mRadius, mRadius);

    if(mRelPos.isKeyOnCurrentFrame() ) {
        p->save();
        p->setBrush(Qt::red);
        p->setPen(QPen(Qt::black, 1.) );
        p->drawEllipse(absPos, 4, 4);
        p->restore();
    }
    if(mode == CanvasMode::MOVE_POINT || (mode == CanvasMode::ADD_POINT && mSelected)) {
        QPen pen = p->pen();
        if(mEndCtrlPt->isVisible() || mode == CanvasMode::ADD_POINT) {
            p->setPen(QPen(Qt::black, 1.5));
            p->drawLine(absPos, mEndCtrlPt->getAbsolutePos());
            p->setPen(QPen(Qt::white, 0.75));
            p->drawLine(absPos, mEndCtrlPt->getAbsolutePos());
        }
        if(mStartCtrlPt->isVisible() || mode == CanvasMode::ADD_POINT) {
            p->setPen(QPen(Qt::black, 1.5));
            p->drawLine(absPos, mStartCtrlPt->getAbsolutePos());
            p->setPen(QPen(Qt::white, 0.75));
            p->drawLine(absPos, mStartCtrlPt->getAbsolutePos());
        }
        p->setPen(pen);
        mEndCtrlPt->draw(p);
        mStartCtrlPt->draw(p);
    }

    if(isCtrlPressed()) {
        QPen pen = p->pen();
        p->setPen(Qt::NoPen);
        p->setBrush(Qt::white);
        p->drawEllipse(absPos, 6., 6.);
        p->setPen(pen);
        p->drawText(QRectF(absPos - QPointF(mRadius, mRadius),
                           absPos + QPointF(mRadius, mRadius)),
                    Qt::AlignCenter,
                    QString::number(mPointId));
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

void PathPoint::updateAfterFrameChanged(int frame)
{
    MovablePoint::updateAfterFrameChanged(frame);
    mEndCtrlPt->updateAfterFrameChanged(frame);
    mStartCtrlPt->updateAfterFrameChanged(frame);
}

void PathPoint::setPointId(int idT) {
    mPathPointAnimators.setName("point " + QString::number(idT) );
    mPointId = idT;
}

PathPointAnimators *PathPoint::getPathPointAnimatorsPtr()
{
    return &mPathPointAnimators;
}

bool PathPoint::isEndCtrlPtEnabled()
{
    return mEndCtrlPtEnabled;
}

bool PathPoint::isStartCtrlPtEnabled()
{
    return mStartCtrlPtEnabled;
}

void PathPoint::setPosAnimatorUpdater(AnimatorUpdater *updater)
{
    MovablePoint::setPosAnimatorUpdater(updater);
    mEndCtrlPt->setPosAnimatorUpdater(updater);
    mStartCtrlPt->setPosAnimatorUpdater(updater);
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

VectorPath *PathPoint::getParentPath()
{
    return mVectorPath;
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
            QPointF newStartPos;
            QPointF newEndPos;
            getCtrlsSymmetricPos(mEndCtrlPt->getAbsolutePos(),
                                 mStartCtrlPt->getAbsolutePos(),
                                 getAbsolutePos(),
                                 &newEndPos,
                                 &newStartPos);
            mStartCtrlPt->setAbsolutePos(newStartPos);
            mEndCtrlPt->setAbsolutePos(newEndPos);
//            QPointF point1 = mEndCtrlPt->getAbsolutePos();
//            point1 = symmetricToAbsPos(point1);
//            QPointF point2 = mStartCtrlPt->getAbsolutePos();
//            qreal len1 = pointToLen(point1);
//            qreal len2 = pointToLen(point2);
//            qreal lenSum = len1 + len2;
//            QPointF newStartCtrlPtPos = (point1*len1 + point2*len2)/lenSum;
//            QPointF newEndCtrlPtPos = symmetricToAbsPos(newStartCtrlPtPos);
//            mStartCtrlPt->setAbsolutePos(newStartCtrlPtPos);
//            mEndCtrlPt->setAbsolutePos(newEndCtrlPtPos);

        } else if(mCtrlsMode == CtrlsMode::CTRLS_SMOOTH) {
            QPointF newStartPos;
            QPointF newEndPos;
            getCtrlsSmoothPos(mEndCtrlPt->getAbsolutePos(),
                              mStartCtrlPt->getAbsolutePos(),
                              getAbsolutePos(),
                              &newEndPos,
                              &newStartPos);
            mStartCtrlPt->setAbsolutePos(newStartPos);
            mEndCtrlPt->setAbsolutePos(newEndPos);
//            QPointF point1 = mEndCtrlPt->getAbsolutePos();
//            point1 = symmetricToAbsPos(point1);
//            QPointF point2 = mStartCtrlPt->getAbsolutePos();
//            qreal len1 = pointToLen(point1);
//            qreal len2 = pointToLen(point2);
//            qreal lenSum = len1 + len2;
//            QPointF point1Rel = mEndCtrlPt->getAbsolutePos() - getAbsolutePos();
//            QPointF point2Rel = mStartCtrlPt->getAbsolutePos() - getAbsolutePos();
//            QPointF newStartDirection =
//                    scalePointToNewLen(
//                        (point1*len1 + point2*len2)/lenSum - getAbsolutePos(),
//                        1.f);
//            qreal startCtrlPtLen =
//                    abs(QPointF::dotProduct(point2Rel, newStartDirection));
//            QPointF newStartCtrlPtPos = newStartDirection*startCtrlPtLen +
//                    getAbsolutePos();
//            qreal endCtrlPtLen =
//                    abs(QPointF::dotProduct(point1Rel, newStartDirection));
//            QPointF newEndCtrlPtPos = -newStartDirection*endCtrlPtLen +
//                    getAbsolutePos();
//            mStartCtrlPt->setAbsolutePos(newStartCtrlPtPos);
//            mEndCtrlPt->setAbsolutePos(newEndCtrlPtPos);
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

void PathPoint::setPointAsNext(PathPoint *pointToSet, bool saveUndoRedo) {
    if(saveUndoRedo) startNewUndoRedoSet();

    if(hasNextPoint()) {
        mNextPoint->setPreviousPoint(NULL, saveUndoRedo);
    }
    setNextPoint(pointToSet, saveUndoRedo);
    if(pointToSet != NULL) {
        pointToSet->setPreviousPoint(this, saveUndoRedo);
    }

    if(saveUndoRedo) finishUndoRedoSet();
}

void PathPoint::setPointAsPrevious(PathPoint *pointToSet, bool saveUndoRedo) {
    if(saveUndoRedo) startNewUndoRedoSet();

    if(hasPreviousPoint()) {
        mPreviousPoint->setNextPoint(NULL, saveUndoRedo);
    }
    setPreviousPoint(pointToSet, saveUndoRedo);
    if(pointToSet != NULL) {
        pointToSet->setNextPoint(this, saveUndoRedo);
    }

    if(saveUndoRedo) finishUndoRedoSet();
}

PathPoint *PathPoint::addPointAbsPos(QPointF absPos)
{
    return mVectorPath->addPointAbsPos(absPos, this);
}

PathPoint *PathPoint::addPoint(PathPoint *pointToAdd)
{
    return mVectorPath->addPoint(pointToAdd, this);
}

bool PathPoint::isEndPoint() {
    return mNextPoint == NULL || mPreviousPoint == NULL;
}
