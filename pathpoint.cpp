#include "pathpoint.h"
#include "Boxes/vectorpath.h"
#include "undoredo.h"
#include "ctrlpoint.h"
#include <QPainter>
#include <QDebug>

PathPoint::PathPoint(VectorPath *vectorPath) :
    MovablePoint(vectorPath, MovablePointType::TYPE_PATH_POINT, 9.5)
{
    mVectorPath = vectorPath;
    mStartCtrlPt = new CtrlPoint(this, true);
    mEndCtrlPt = new CtrlPoint(this, false);
    mStartCtrlPt->incNumberPointers();
    mEndCtrlPt->incNumberPointers();

    mStartCtrlPt->setOtherCtrlPt(mEndCtrlPt);
    mEndCtrlPt->setOtherCtrlPt(mStartCtrlPt);

    mStartCtrlPt->hide();
    mEndCtrlPt->hide();

    //PathPointUpdater *updater = new PathPointUpdater(vectorPath);
    //setPosAnimatorUpdater(updater);

    mPathPointAnimators.setAllVars(this,
                                   mEndCtrlPt->getRelativePosAnimatorPtr(),
                                   mStartCtrlPt->getRelativePosAnimatorPtr(),
                                   getRelativePosAnimatorPtr());

    mPathPointAnimators.incNumberPointers();

    mRelPos.setTraceKeyOnCurrentFrame(true);
}

PathPoint::~PathPoint()
{
    mStartCtrlPt->decNumberPointers();
    mEndCtrlPt->decNumberPointers();
}

void PathPoint::applyTransform(QMatrix transform)
{
    mStartCtrlPt->applyTransform(transform);
    mEndCtrlPt->applyTransform(transform);
    MovablePoint::applyTransform(transform);
}

void PathPoint::loadFromSql(int pathPointId, int movablePointId) {
    MovablePoint::loadFromSql(movablePointId);
    QSqlQuery query;
    QString queryStr = "SELECT * FROM pathpoint WHERE id = " +
            QString::number(pathPointId);
    if(query.exec(queryStr)) {
        query.next();
        int idisfirst = query.record().indexOf("isfirst");
        int idstartctrlptid = query.record().indexOf("startctrlptid");
        int idendctrlptid = query.record().indexOf("endctrlptid");
        int idstartpointenabled = query.record().indexOf("startpointenabled");
        int idendpointenabled = query.record().indexOf("endpointenabled");
        int idctrlsmode = query.record().indexOf("ctrlsmode");

        mSeparatePathPoint = query.value(idisfirst).toBool();
        mStartCtrlPtEnabled = query.value(idstartpointenabled).toBool();
        mEndCtrlPtEnabled = query.value(idendpointenabled).toBool();
        mCtrlsMode = static_cast<CtrlsMode>(query.value(idctrlsmode).toInt() );

        mStartCtrlPt->loadFromSql(query.value(idstartctrlptid).toInt());
        mEndCtrlPt->loadFromSql(query.value(idendctrlptid).toInt());
    } else {
        qDebug() << "Could not load pathpoint with id " << pathPointId;
    }
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
    MovablePoint::finishTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->MovablePoint::finishTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->MovablePoint::finishTransform();
    }
}

void PathPoint::moveByRel(QPointF relTranslation)
{
    MovablePoint::moveByRel(relTranslation);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->MovablePoint::moveByRel(relTranslation);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->MovablePoint::moveByRel(relTranslation);
    }
}

void PathPoint::moveByAbs(QPointF absTranslatione) {
    MovablePoint::moveByAbs(absTranslatione);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->MovablePoint::moveByAbs(absTranslatione);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->MovablePoint::moveByAbs(absTranslatione);
    }
}

void PathPoint::reversePointsDirection() {
    PathPoint *nextT = mNextPoint;
    mNextPoint = mPreviousPoint;
    mPreviousPoint = nextT;
    mEndCtrlPt->setIsStartCtrlPt(true);
    mStartCtrlPt->setIsStartCtrlPt(false);
    CtrlPoint *endPointT = mEndCtrlPt;
    mEndCtrlPt = mStartCtrlPt;
    mStartCtrlPt = endPointT;
    if(nextT == NULL) return;
    nextT->reversePointsDirection();
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

void PathPoint::removeFromVectorPath()
{
    mVectorPath->removePoint(this);
}

void PathPoint::removeApproximate()
{
    mVectorPath->deletePointAndApproximate(this);
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

MovablePoint *PathPoint::getPointAtAbsPos(QPointF absPos,
                                          const CanvasMode &canvasMode)
{
    if(canvasMode == CanvasMode::MOVE_POINT) {
        if(mStartCtrlPt->isPointAtAbsPos(absPos)) {
            return mStartCtrlPt;
        } else if (mEndCtrlPt->isPointAtAbsPos(absPos)) {
            return mEndCtrlPt;
        }
    } else {
        if(!isEndPoint()) {
            return NULL;
        }
    }
    if (isPointAtAbsPos(absPos)) {
        return this;
    }
    return NULL;
}

#include <QSqlError>
void PathPoint::saveToSql(QSqlQuery *query, int boundingBoxId)
{
    int movablePtId = MovablePoint::saveToSql(query);
    int startPtId = mStartCtrlPt->saveToSql(query);
    int endPtId = mEndCtrlPt->saveToSql(query);
    QString isFirst = ( (mSeparatePathPoint) ? "1" : "0" );
    QString isEnd = ( (isEndPoint()) ? "1" : "0" );
    if(!query->exec(QString("INSERT INTO pathpoint (isfirst, isendpoint, "
                "movablepointid, startctrlptid, endctrlptid, boundingboxid, "
                "ctrlsmode, startpointenabled, endpointenabled) "
                "VALUES (%1, %2, %3, %4, %5, %6, %7, %8, %9)").
                arg(isFirst).
                arg(isEnd).
                arg(movablePtId).
                arg(startPtId).
                arg(endPtId).
                arg(boundingBoxId).
                arg(mCtrlsMode).
                arg(mStartCtrlPtEnabled).
                arg(mEndCtrlPtEnabled) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }
    if(mNextPoint != NULL) {
        if(!mNextPoint->isSeparatePathPoint()) {
            mNextPoint->saveToSql(query, boundingBoxId);
        }
    }
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

QPointF PathPoint::getStartCtrlPtAbsPos() const
{
    return mapRelativeToAbsolute(getStartCtrlPtValue());
}

QPointF PathPoint::getStartCtrlPtValue() const
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
        pointToUpdate->moveToAbsWithoutUpdatingTheOther(symmetricToAbsPos(changedPointPos));
    } else if(mCtrlsMode == CtrlsMode::CTRLS_SMOOTH) {
        if(!isPointZero(changedPointPos) ) {
            pointToUpdate->moveToAbsWithoutUpdatingTheOther(
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

QPointF PathPoint::getEndCtrlPtAbsPos() {
    return mapRelativeToAbsolute(getEndCtrlPtValue());
}

QPointF PathPoint::getEndCtrlPtValue() const
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

void PathPoint::draw(QPainter *p, const CanvasMode &mode)
{
    if(mSelected) {
        p->setBrush(QColor(0, 200, 255));
    } else {
        p->setBrush(QColor(170, 240, 255));
    }
    QPointF absPos = getAbsolutePos();
    p->drawEllipse(absPos,
                   mRadius - 2, mRadius - 2);

    if(mRelPos.isKeyOnCurrentFrame() ) {
        p->save();
        p->setBrush(Qt::red);
        p->setPen(QPen(Qt::black, 1.) );
        p->drawEllipse(absPos, 4, 4);
        p->restore();
    }
    if((mode == CanvasMode::MOVE_POINT &&
       (isNeighbourSelected() || BoxesGroup::getCtrlsAlwaysVisible() ) ) ||
       (mode == CanvasMode::ADD_POINT && mSelected) ) {
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

PathPoint *PathPoint::getPreviousPoint() {
    return mPreviousPoint;
}

PathPoint *PathPoint::getConnectedSeparatePathPoint() {
    if(isSeparatePathPoint() ||
       mPreviousPoint == NULL) return this;
    return mPreviousPoint->getConnectedSeparatePathPoint();
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
    if(enabled == mEndCtrlPtEnabled) return;
    if(mEndCtrlPtEnabled) {
        setCtrlsMode(CtrlsMode::CTRLS_CORNER);
        mEndCtrlPt->removeAnimations();
    }
    mEndCtrlPt->setRelativePos(getRelativePos());
    mEndCtrlPtEnabled = enabled;
    updateEndCtrlPtVisibility();
    mVectorPath->schedulePathUpdate();
}

void PathPoint::setStartCtrlPtEnabled(bool enabled)
{
    if(enabled == mStartCtrlPtEnabled) return;
    if(mStartCtrlPtEnabled) {
        setCtrlsMode(CtrlsMode::CTRLS_CORNER);
        mStartCtrlPt->removeAnimations();
    }
    mStartCtrlPt->setRelativePos(getRelativePos());
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

int PathPoint::getPointId() {
    return mPointId;
}

PathPointValues PathPoint::getPointValues() const
{
//    if(mEditingShape) {
//        return mBasisShapeSavedValues;
//    }
    return PathPointValues(getStartCtrlPtValue(),
                           getRelativePos(),
                           getEndCtrlPtValue() );
}

void PathPoint::removeShapeValues(VectorPathShape *shape) {
    for(int i = 0; i < mShapeValues.count(); i++) {
        if(mShapeValues.at(i)->getParentShape() == shape) {
            delete mShapeValues.takeAt(i);
            return;
        }
    }
}

void PathPoint::addShapeValues(VectorPathShape *shape) {
    mShapeValues << new PointShapeValues(shape, mPointId);
}

void PathPoint::setPointValues(const PathPointValues &values) {
    setRelativePos(values.pointRelPos, false);
    mEndCtrlPt->setRelativePos(values.endRelPos, false);
    mStartCtrlPt->setRelativePos(values.startRelPos, false);
}

void PathPoint::editShape(VectorPathShape *shape)
{
    mBasisShapeSavedValues = getPointValues();
    mEditingShape = true;
    foreach(PointShapeValues *pointShapeValues, mShapeValues) {
        if(pointShapeValues->getParentShape() == shape) {
            if(shape->isRelative()) {
                setPointValues(pointShapeValues->getValues() + mBasisShapeSavedValues);
            } else {
                setPointValues(pointShapeValues->getValues());
            }
            return;
        }
    }
}

void PathPoint::finishEditingShape(VectorPathShape *shape)
{
    mEditingShape = false;
    savePointValuesToShapeValues(shape);
    setPointValues(mBasisShapeSavedValues);
}

void PathPoint::cancelEditingShape()
{
    mEditingShape = false;
    setPointValues(mBasisShapeSavedValues);
}

void PathPoint::saveInitialPointValuesToShapeValues(VectorPathShape *shape)
{
    if(shape->isRelative()) {
        foreach(PointShapeValues *pointShapeValues, mShapeValues) {
            if(pointShapeValues->getParentShape() == shape) {
                pointShapeValues->setPointValues(PathPointValues(QPointF(0., 0.),
                                                                 QPointF(0., 0.),
                                                                 QPointF(0., 0.)));
                return;
            }
        }
    } else {
        foreach(PointShapeValues *pointShapeValues, mShapeValues) {
            if(pointShapeValues->getParentShape() == shape) {
                pointShapeValues->setPointValues(getPointValues());
                return;
            }
        }
    }
}

void PathPoint::makeDuplicate(MovablePoint *targetPoint) {
    MovablePoint::makeDuplicate(targetPoint);
    PathPoint *target = (PathPoint*)targetPoint;
    target->setCtrlsMode(mCtrlsMode);
    target->setEndCtrlPtEnabled(mEndCtrlPtEnabled);
    target->setStartCtrlPtEnabled(mStartCtrlPtEnabled);
    target->duplicateCtrlPointsFrom(mEndCtrlPt,
                                    mStartCtrlPt);
}

void PathPoint::duplicateCtrlPointsFrom(CtrlPoint *endPt,
                                        CtrlPoint *startPt) {
    endPt->makeDuplicate(mEndCtrlPt);
    startPt->makeDuplicate(mStartCtrlPt);
}

void PathPoint::savePointValuesToShapeValues(VectorPathShape *shape)
{
    PathPointValues values;
    if(shape->isRelative()) {
        values = getPointValues() - mBasisShapeSavedValues;
    } else {
        values = getPointValues();
    }
    foreach(PointShapeValues *pointShapeValues, mShapeValues) {
        if(pointShapeValues->getParentShape() == shape) {
            addUndoRedo(new ChangePointShapeValuesUndoRedo(
                            pointShapeValues,
                            pointShapeValues->getValues(),
                            values));
            pointShapeValues->setPointValues(values);
            return;
        }
    }
}

PathPointValues PathPoint::getShapesInfluencedPointValues() const
{
    if(mShapeValues.isEmpty() ) return getPointValues();
    PathPointValues relativeValues = PathPointValues(QPointF(0., 0.),
                                                     QPointF(0., 0.),
                                                     QPointF(0., 0.));
    PathPointValues absValues = PathPointValues(QPointF(0., 0.),
                                                QPointF(0., 0.),
                                                QPointF(0., 0.));

    qreal nAbs = 0.;
    foreach(PointShapeValues *pointShapeValues, mShapeValues) {
        VectorPathShape *parentShape = pointShapeValues->getParentShape();
        qreal infl = parentShape->getCurrentInfluence();
        if(parentShape->isRelative()) {
            relativeValues += pointShapeValues->getValues()*infl;
        } else {
            absValues += pointShapeValues->getValues()*infl;
            nAbs += infl;
        }
    }
    if(nAbs < 1.) {
        if(mEditingShape) {
            absValues += mBasisShapeSavedValues*(1. - nAbs);
        } else {
            absValues += getPointValues()*(1. - nAbs);
        }
        nAbs = 1.;
    }
    return absValues/nAbs + relativeValues;
}

CtrlsMode PathPoint::getCurrentCtrlsMode()
{
    return mCtrlsMode;
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

bool PathPoint::isNeighbourSelected() {
    bool nextSelected = (mNextPoint == NULL) ?
                false : mNextPoint->isSelected();
    bool prevSelected = (mPreviousPoint == NULL) ?
                false : mPreviousPoint->isSelected();
    return isSelected() || nextSelected || prevSelected;
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
        } else {
            return;
        }

        setCtrlPtEnabled(true, true);
        setCtrlPtEnabled(true, false);
        mVectorPath->schedulePathUpdate();
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
    if(hasNextPoint()) {
        mNextPoint->setPreviousPoint(NULL, saveUndoRedo);
    }
    setNextPoint(pointToSet, saveUndoRedo);
    if(pointToSet != NULL) {
        pointToSet->setPreviousPoint(this, saveUndoRedo);
    }
}

void PathPoint::setPointAsPrevious(PathPoint *pointToSet, bool saveUndoRedo) {
    if(hasPreviousPoint()) {
        mPreviousPoint->setNextPoint(NULL, saveUndoRedo);
    }
    setPreviousPoint(pointToSet, saveUndoRedo);
    if(pointToSet != NULL) {
        pointToSet->setNextPoint(this, saveUndoRedo);
    }
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

PathPointValues operator-(const PathPointValues &ppv1, const PathPointValues &ppv2)
{
    return PathPointValues(ppv1.startRelPos - ppv2.startRelPos,
                           ppv1.pointRelPos - ppv2.pointRelPos,
                           ppv1.endRelPos - ppv2.endRelPos);
}

PathPointValues operator+(const PathPointValues &ppv1, const PathPointValues &ppv2)
{
    return PathPointValues(ppv1.startRelPos + ppv2.startRelPos,
                           ppv1.pointRelPos + ppv2.pointRelPos,
                           ppv1.endRelPos + ppv2.endRelPos);
}

PathPointValues operator/(const PathPointValues &ppv, const qreal &val)
{
    return PathPointValues(ppv.startRelPos / val,
                           ppv.pointRelPos / val,
                           ppv.endRelPos / val);
}

PathPointValues operator*(const qreal &val, const PathPointValues &ppv)
{
    return ppv*val;
}

PathPointValues operator*(const PathPointValues &ppv, const qreal &val)
{
    return PathPointValues(ppv.startRelPos * val,
                           ppv.pointRelPos * val,
                           ppv.endRelPos * val);
}
