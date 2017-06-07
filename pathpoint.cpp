#include "pathpoint.h"
#include "Animators/singlepathanimator.h"
#include "undoredo.h"
#include "ctrlpoint.h"
#include <QPainter>
#include <QDebug>
#include "edge.h"
#include "Animators/pathanimator.h"
#include "canvas.h"

PathPoint::PathPoint(SinglePathAnimator *parentAnimator) :
    MovablePoint(parentAnimator->getParentPathAnimator()->getParentBox(),
                 MovablePointType::TYPE_PATH_POINT, 9.5) {
    mParentPath = parentAnimator;
    mStartCtrlPt = new CtrlPoint(this, true);
    mEndCtrlPt = new CtrlPoint(this, false);

    mStartCtrlPt->setOtherCtrlPt(mEndCtrlPt);
    mEndCtrlPt->setOtherCtrlPt(mStartCtrlPt);

    mStartCtrlPt->hide();
    mEndCtrlPt->hide();

    //PathPointUpdater *updater = new PathPointUpdater(vectorPath);
    //setPosAnimatorUpdater(updater);

    mPathPointAnimators->setAllVars(this,
                                   mEndCtrlPt,
                                   mStartCtrlPt);

    anim_setTraceKeyOnCurrentFrame(true);
}

PathPoint::~PathPoint() {
}

void PathPoint::setParentPath(SinglePathAnimator *path) {
    mParentPath = path;
}

void PathPoint::applyTransform(const QMatrix &transform) {
    mStartCtrlPt->applyTransform(transform);
    mEndCtrlPt->applyTransform(transform);
    MovablePoint::applyTransform(transform);
}

void PathPoint::prp_loadFromSql(const int &movablePointId) {
    MovablePoint::prp_loadFromSql(movablePointId);
    QSqlQuery query;
    QString queryStr = "SELECT * FROM pathpoint WHERE qpointfanimatorid = " +
            QString::number(movablePointId);
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

        mStartCtrlPt->prp_loadFromSql(query.value(idstartctrlptid).toInt());
        mEndCtrlPt->prp_loadFromSql(query.value(idendctrlptid).toInt());
    } else {
        qDebug() << "Could not load pathpoint with id " << movablePointId;
    }
}

void PathPoint::startTransform() {
    MovablePoint::startTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->MovablePoint::startTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->MovablePoint::startTransform();
    }
}

void PathPoint::saveTransformPivotAbsPos(const QPointF &absPivot) {
    MovablePoint::saveTransformPivotAbsPos(absPivot);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->saveTransformPivotAbsPos(absPivot);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->saveTransformPivotAbsPos(absPivot);
    }
}

void PathPoint::rotateRelativeToSavedPivot(const qreal &rot) {
    MovablePoint::rotateRelativeToSavedPivot(rot);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->rotateRelativeToSavedPivot(rot);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->rotateRelativeToSavedPivot(rot);
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

void PathPoint::finishTransform() {
    MovablePoint::finishTransform();
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->MovablePoint::finishTransform();
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->MovablePoint::finishTransform();
    }
}

void PathPoint::moveByRel(const QPointF &relTranslation) {
    MovablePoint::moveByRel(relTranslation);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->MovablePoint::moveByRel(relTranslation);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->MovablePoint::moveByRel(relTranslation);
    }
}

void PathPoint::moveByAbs(const QPointF &absTranslatione) {
    MovablePoint::moveByAbs(absTranslatione);
    if(!mStartCtrlPt->isSelected()) {
        mStartCtrlPt->MovablePoint::moveByAbs(absTranslatione);
    }
    if(!mEndCtrlPt->isSelected()) {
        mEndCtrlPt->MovablePoint::moveByAbs(absTranslatione);
    }
}

void PathPoint::reversePointsDirectionStartingFromThis(
        const bool &saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new ReversePointsDirectionUndoRedo(this));
    }
    reversePointsDirection();
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

void PathPoint::reversePointsDirectionReverse() {
    PathPoint *prevT = mPreviousPoint;
    mPreviousPoint = mNextPoint;
    mNextPoint = prevT;
    mEndCtrlPt->setIsStartCtrlPt(true);
    mStartCtrlPt->setIsStartCtrlPt(false);
    CtrlPoint *endPointT = mEndCtrlPt;
    mEndCtrlPt = mStartCtrlPt;
    mStartCtrlPt = endPointT;
    if(prevT == NULL) return;
    prevT->reversePointsDirectionReverse();
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

void PathPoint::removeFromVectorPath() {
    mParentPath->removePoint(this);
}

void PathPoint::removeApproximate() {
    mParentPath->deletePointAndApproximate(this);
}

void PathPoint::rectPointsSelection(QRectF absRect,
                                    QList<MovablePoint*> *list) {
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

MovablePoint *PathPoint::getPointAtAbsPos(const QPointF &absPos,
                                          const CanvasMode &canvasMode,
                                          const qreal &canvasScaleInv)
{
    if(canvasMode == CanvasMode::MOVE_POINT) {
        if(mStartCtrlPt->isPointAtAbsPos(absPos,
                                         canvasScaleInv)) {
            return mStartCtrlPt;
        } else if (mEndCtrlPt->isPointAtAbsPos(absPos,
                                               canvasScaleInv)) {
            return mEndCtrlPt;
        }
    } else {
        if(!isEndPoint()) {
            return NULL;
        }
    }
    if (isPointAtAbsPos(absPos,
                        canvasScaleInv)) {
        return this;
    }
    return NULL;
}

#include <QSqlError>
int PathPoint::prp_saveToSql(QSqlQuery *query, const int &boundingBoxId) {
    int movablePtId = MovablePoint::prp_saveToSql(query);
    int startPtId = mStartCtrlPt->prp_saveToSql(query);
    int endPtId = mEndCtrlPt->prp_saveToSql(query);
    QString isFirst = ( (mSeparatePathPoint) ? "1" : "0" );
    QString isEnd = ( (isEndPoint()) ? "1" : "0" );
    if(!query->exec(QString("INSERT INTO pathpoint (isfirst, isendpoint, "
                "qpointfanimatorid, startctrlptid, endctrlptid, boundingboxid, "
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
            mNextPoint->prp_saveToSql(query, boundingBoxId);
        }
    }
    return movablePtId;
}

QPointF PathPoint::symmetricToAbsPos(QPointF absPosToMirror) {
    return symmetricToPos(absPosToMirror, getAbsolutePos());
}

QPointF PathPoint::symmetricToAbsPosNewLen(QPointF absPosToMirror,
                                           qreal newLen) {
    return symmetricToPosNewLen(absPosToMirror, getAbsolutePos(), newLen);
}

void PathPoint::moveStartCtrlPtToAbsPos(QPointF startCtrlPt) {
    mStartCtrlPt->moveToAbs(startCtrlPt);
}

void PathPoint::moveEndCtrlPtToRelPos(QPointF endCtrlPt) {
    mEndCtrlPt->setRelativePos(endCtrlPt);
}

void PathPoint::moveStartCtrlPtToRelPos(QPointF startCtrlPt) {
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
        pointToUpdate->moveToAbsWithoutUpdatingTheOther(
                    symmetricToAbsPos(changedPointPos));
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

void PathPoint::moveEndCtrlPtToAbsPos(QPointF endCtrlPt) {
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

void PathPoint::drawSk(SkCanvas *canvas,
                     const CanvasMode &mode,
                     const qreal &invScale) {
    canvas->save();
    SkPoint absPos = QPointFToSkPoint(getAbsolutePos());
    if(mSelected) {
        drawOnAbsPosSk(canvas,
                     absPos,
                     invScale,
                     0, 200, 255);
    } else {
        drawOnAbsPosSk(canvas,
                     absPos,
                     invScale,
                     170, 240, 255);
    }

    if((mode == CanvasMode::MOVE_POINT &&
        (isNeighbourSelected() ||
         BoxesGroup::getCtrlsAlwaysVisible() ) ) ||
            (mode == CanvasMode::ADD_POINT && mSelected)) {
        SkPaint paint;
        paint.setAntiAlias(true);
        if(mEndCtrlPt->isVisible() || mode == CanvasMode::ADD_POINT) {
            SkPoint endAbsPos = QPointFToSkPoint(
                        mEndCtrlPt->getAbsolutePos());
            paint.setColor(SK_ColorBLACK);
            paint.setStrokeWidth(1.5*invScale);
            paint.setStyle(SkPaint::kStroke_Style);

            canvas->drawLine(absPos, endAbsPos, paint);
            paint.setColor(SK_ColorWHITE);
            paint.setStrokeWidth(0.75*invScale);
            canvas->drawLine(absPos, endAbsPos, paint);
        }
        if(mStartCtrlPt->isVisible() || mode == CanvasMode::ADD_POINT) {
            SkPoint startAbsPos = QPointFToSkPoint(
                        mStartCtrlPt->getAbsolutePos());
            paint.setColor(SK_ColorBLACK);
            paint.setStrokeWidth(1.5*invScale);
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawLine(absPos, startAbsPos, paint);

            paint.setColor(SK_ColorWHITE);
            paint.setStrokeWidth(0.75*invScale);
            canvas->drawLine(absPos, startAbsPos, paint);
        }
        mEndCtrlPt->drawSk(canvas, invScale);
        mStartCtrlPt->drawSk(canvas, invScale);
    }

//    if(isCtrlPressed()) {
//        QPen pen = p->pen();
//        p->setPen(Qt::NoPen);
//        p->setBrush(Qt::white);
//        drawCosmeticEllipse(p, absPos,
//                            6., 6.);
//        p->setPen(pen);
//        canvas->save();
//        SkMatrix trans = canvas->getTotalMatrix();
//        canvas->resetMatrix();
//        absPos = trans.mapXY(absPos.x(), absPos.y());
//        SkString text;
//        text.appendS32(mPointId);
//        canvas->drawString(QRectF(absPos - QPointF(mRadius, mRadius),
//                           absPos + QPointF(mRadius, mRadius)),
//                    Qt::AlignCenter,
//                    SkString::appendS32(number(mPointId));
//        canvas->restore();
//    }
    canvas->restore();
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

void PathPoint::setNextPoint(PathPoint *nextPoint, bool saveUndoRedo) {
    if(saveUndoRedo) {
        SetNextPointUndoRedo *undoRedo = new SetNextPointUndoRedo(this,
                                                                  mNextPoint,
                                                                  nextPoint);
        addUndoRedo(undoRedo);
    }
    mNextPoint = nextPoint;
    if(mNextPoint == NULL) {
        if(mNextEdge.get() != NULL) {
            mNextEdge.reset();
        }
    } else {
        if(mNextEdge.get() == NULL) {
            mNextEdge = (new VectorPathEdge(this, mNextPoint))->ref<VectorPathEdge>();
        } else {
            mNextEdge->setPoint2(mNextPoint);
        }
    }
    updateEndCtrlPtVisibility();
    prp_callUpdater();
    //mParentPath->schedulePathUpdate();
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

void PathPoint::setEndCtrlPtEnabled(bool enabled,
                                    bool saveUndoRedo)
{
    if(enabled == mEndCtrlPtEnabled) return;
    if(mEndCtrlPtEnabled) {
        setCtrlsMode(CtrlsMode::CTRLS_CORNER, saveUndoRedo);
        mEndCtrlPt->removeAnimations();
    }
    //mEndCtrlPt->setRelativePos(getRelativePos());
    mEndCtrlPtEnabled = enabled;
    updateEndCtrlPtVisibility();
    prp_callUpdater();
    //mParentPath->schedulePathUpdate();
}

void PathPoint::setStartCtrlPtEnabled(bool enabled,
                                      bool saveUndoRedo)
{
    if(enabled == mStartCtrlPtEnabled) return;
    if(mStartCtrlPtEnabled) {
        setCtrlsMode(CtrlsMode::CTRLS_CORNER, saveUndoRedo);
        mStartCtrlPt->removeAnimations();
    }
    //mStartCtrlPt->setRelativePos(getRelativePos());
    mStartCtrlPtEnabled = enabled;
    updateStartCtrlPtVisibility();
    prp_callUpdater();
    //mParentPath->schedulePathUpdate();
}

void PathPoint::updateAfterFrameChanged(const int &frame)
{
    MovablePoint::updateAfterFrameChanged(frame);
    mEndCtrlPt->updateAfterFrameChanged(frame);
    mStartCtrlPt->updateAfterFrameChanged(frame);
}

void PathPoint::setPointId(int idT) {
    mPathPointAnimators->prp_setName("point " + QString::number(idT) );
    mPointId = idT;
}

int PathPoint::getPointId() {
    return mPointId;
}

PathPointValues PathPoint::getPointValues() const {
    return PathPointValues(getStartCtrlPtValue(),
                           getRelativePos(),
                           getEndCtrlPtValue() );
}

void PathPoint::setPointValues(const PathPointValues &values) {
    setRelativePos(values.pointRelPos, false);
    mEndCtrlPt->setRelativePos(values.endRelPos, false);
    mStartCtrlPt->setRelativePos(values.startRelPos, false);
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

CtrlsMode PathPoint::getCurrentCtrlsMode()
{
    return mCtrlsMode;
}

PathPointAnimators *PathPoint::getPathPointAnimatorsPtr() {
    return mPathPointAnimators.data();
}

bool PathPoint::isEndCtrlPtEnabled()
{
    return mEndCtrlPtEnabled;
}

bool PathPoint::isStartCtrlPtEnabled()
{
    return mStartCtrlPtEnabled;
}

void PathPoint::setPosAnimatorUpdater(AnimatorUpdater *updater) {
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

SinglePathAnimator *PathPoint::getParentPath() {
    return mParentPath;
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
    }
    setCtrlPtEnabled(true, true, saveUndoRedo);
    setCtrlPtEnabled(true, false, saveUndoRedo);
    //mParentPath->schedulePathUpdate();
    prp_callUpdater();
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
    //mParentPath->schedulePathUpdate();
    prp_callUpdater();
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

PathPoint *PathPoint::addPointAbsPos(QPointF absPos) {
    return mParentPath->addPointAbsPos(absPos, this);
}

PathPoint *PathPoint::addPoint(PathPoint *pointToAdd)
{
    return mParentPath->addPoint(pointToAdd, this);
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

PathPointValues operator/(const PathPointValues &ppv, const qreal &val) {
    qreal invVal = 1.f/val;
    return PathPointValues(ppv.startRelPos * invVal,
                           ppv.pointRelPos * invVal,
                           ppv.endRelPos * invVal);
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

PathPointAnimators::PathPointAnimators() : ComplexAnimator() {
    prp_setName("point");
}

void PathPointAnimators::setAllVars(PathPoint *parentPathPointT, QPointFAnimator *endPosAnimatorT, QPointFAnimator *startPosAnimatorT) {
    mParentPathPoint = parentPathPointT;
    mParentPathPoint->prp_setName("point pos");
    mEndPosAnimator = endPosAnimatorT;
    mEndPosAnimator->prp_setName("ctrl pt 1 pos");
    mStartPosAnimator = startPosAnimatorT;
    mStartPosAnimator->prp_setName("ctrl pt 2 pos");

    ca_addChildAnimator(mParentPathPoint);
    ca_addChildAnimator(mEndPosAnimator);
    ca_addChildAnimator(mStartPosAnimator);
}
