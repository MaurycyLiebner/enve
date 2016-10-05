#include "vectorpath.h"
#include <QPainter>
#include "canvas.h"
#include <QDebug>
#include "undoredo.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "pathpivot.h"

VectorPath::VectorPath(BoxesGroup *group) :
    BoundingBox(group,
                BoundingBoxType::TYPE_VECTOR_PATH)
{
    addActiveAnimator(&mPathAnimator);
    addActiveAnimator(&mFillPaintSettings);
    addActiveAnimator(&mStrokeSettings);
    mAnimatorsCollection.addAnimator(&mPathAnimator);
    mAnimatorsCollection.addAnimator(&mFillPaintSettings);
    mAnimatorsCollection.addAnimator(&mStrokeSettings);

    mFillGradientPoints.initialize(this);
    mFillGradientPoints.incNumberPointers();
    mStrokeGradientPoints.initialize(this);
    mStrokeGradientPoints.incNumberPointers();

    mFillPaintSettings.setGradientPoints(&mFillGradientPoints);
    mStrokeSettings.setGradientPoints(&mStrokeGradientPoints);

    mStrokeSettings.setLineWidthUpdaterTarget(this);
}

VectorPath::VectorPath(int boundingBoxId,
                        BoxesGroup *parent) :
    BoundingBox(boundingBoxId,
                parent, TYPE_VECTOR_PATH) {
    addActiveAnimator(&mPathAnimator);
    mAnimatorsCollection.addAnimator(&mPathAnimator);

    QSqlQuery query;
    QString queryStr = "SELECT * FROM vectorpath WHERE boundingboxid = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr) ) {
        query.next();
        int idId = query.record().indexOf("id");
        int idfillgradientstartid = query.record().indexOf("fillgradientstartid");
        int idfillgradientendid = query.record().indexOf("fillgradientendid");
        int idstrokegradientstartid = query.record().indexOf("strokegradientstartid");
        int idstrokegradientendid = query.record().indexOf("strokegradientendid");
        int idfillsettingsid = query.record().indexOf("fillsettingsid");
        int idstrokesettingsid = query.record().indexOf("strokesettingsid");


        int vectorPathId = query.value(idId).toInt();
        int fillGradientStartId = query.value(idfillgradientstartid).toInt();
        int fillGradientEndId = query.value(idfillgradientendid).toInt();
        int strokeGradientStartId = query.value(idstrokegradientstartid).toInt();
        int strokeGradientEndId = query.value(idstrokegradientendid).toInt();
        int fillSettingsId = query.value(idfillsettingsid).toInt();
        int strokeSettingsId = query.value(idstrokesettingsid).toInt();

        loadPointsFromSql(vectorPathId);
        query.exec("CREATE TABLE vectorpath "
                   "(id INTEGER PRIMARY KEY, "
                   "fillgradientstartid INTEGER, "
                   "fillgradientendid INTEGER, "
                   "strokegradientstartid INTEGER, "
                   "strokegradientendid INTEGER, "
                   "boundingboxid INTEGER, "
                   "fillsettingsid INTEGER, "
                   "strokesettingsid INTEGER, "
                   "FOREIGN KEY(fillgradientstartid) REFERENCES movablepoint(id), "
                   "FOREIGN KEY(fillgradientendid) REFERENCES movablepoint(id), "
                   "FOREIGN KEY(strokegradientstartid) REFERENCES movablepoint(id), "
                   "FOREIGN KEY(strokegradientendid) REFERENCES movablepoint(id), "
                   "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id), "
                   "FOREIGN KEY(fillsettingsid) REFERENCES paintsettings(id), "
                   "FOREIGN KEY(strokesettingsid) REFERENCES strokesettings(id) )");
        mFillGradientPoints.initialize(this, fillGradientStartId,
                                       fillGradientEndId);
        mStrokeGradientPoints.initialize(this, strokeGradientStartId,
                                         strokeGradientEndId);

        GradientWidget *gradientWidget =
                mMainWindow->getFillStrokeSettings()->getGradientWidget();

        mFillPaintSettings = PaintSettings(fillSettingsId, gradientWidget);
        mStrokeSettings = StrokeSettings::createStrokeSettingsFromSql(
                    strokeSettingsId, gradientWidget);
    } else {
        qDebug() << "Could not load vectorpath with id " << boundingBoxId;
    }
}

void VectorPath::loadPointsFromSql(int vectorPathId) {
    QSqlQuery query;
    QString queryStr = QString("SELECT id, isfirst, isendpoint, movablepointid "
                               "FROM pathpoint WHERE vectorpathid = %1 "
                               "ORDER BY id ASC").arg(vectorPathId);
    if(query.exec(queryStr) ) {
        int idisfirst = query.record().indexOf("isfirst");
        int idisendpoint = query.record().indexOf("isendpoint");
        int idmovablepointid = query.record().indexOf("movablepointid");
        int idId = query.record().indexOf("id");
        PathPoint *firstPoint = NULL;
        PathPoint *lastPoint = NULL;
        while(query.next()) {
            int id = query.value(idId).toInt();
            bool isfirst = query.value(idisfirst).toBool();
            bool isendpoint = query.value(idisendpoint).toBool();
            int movablepointid = query.value(idmovablepointid).toInt();

            PathPoint *newPoint = new PathPoint(movablepointid, id, this);
            appendToPointsList(newPoint, false);
            if(lastPoint != NULL) {
                if(isfirst && firstPoint != NULL) {
                    lastPoint->setPointAsNext(firstPoint, false);
                } else if(!isfirst) {
                    lastPoint->setPointAsNext(newPoint, false);
                }
            }
            if(isfirst) {
                addPointToSeparatePaths(newPoint, false);
                if(isendpoint) {
                    firstPoint = NULL;
                } else {
                    firstPoint = newPoint;
                }
            }
            lastPoint = newPoint;
        }
        if(lastPoint != NULL && firstPoint != NULL) {
            lastPoint->setPointAsNext(firstPoint, false);
        }
    } else {
        qDebug() << "Could not load points for vectorpath with id " << vectorPathId;
    }
}

void VectorPath::clearAll()
{
    foreach(PathPoint *point, mPoints) {
        point->clearAll();
        delete point;
    }
    mStrokeGradientPoints.clearAll();
    mFillGradientPoints.clearAll();
}

qreal distBetweenTwoPoints(QPointF point1, QPointF point2) {
    QPointF dPoint = point1 - point2;
    return sqrt(dPoint.x()*dPoint.x() + dPoint.y()*dPoint.y());
}

qreal VectorPath::findPercentForPoint(QPointF point,
                                      qreal minPercent, qreal maxPercent) {
    qreal smallestStep = 0.00001f;
    QPointF nearestPoint;
    qreal smallestDist = 1000000.f;
    qreal nearestPercent = minPercent;
    qreal percentStep = (maxPercent - minPercent)*0.01f;
    if(percentStep < smallestStep) return (maxPercent + minPercent)*0.5f;
    qreal currPercent = minPercent;
    for(int i = 0; i < 100; i++) {
        QPointF testPoint = mMappedPath.pointAtPercent(currPercent);
        qreal dist = distBetweenTwoPoints(testPoint, point);
        if(dist < smallestDist) {
            smallestDist = dist;
            nearestPoint = testPoint;
            nearestPercent = currPercent;
        }
        currPercent += percentStep;
    }
    return findPercentForPoint(point, nearestPercent - percentStep,
                               nearestPercent + percentStep);
}

qreal VectorPath::findPercentForPointEditPath(QPointF point,
                                      qreal minPercent, qreal maxPercent) {
    qreal smallestStep = 0.00001f;
    QPointF nearestPoint;
    qreal smallestDist = 1000000.f;
    qreal nearestPercent = minPercent;
    qreal percentStep = (maxPercent - minPercent)*0.01f;
    if(percentStep < smallestStep) return (maxPercent + minPercent)*0.5f;
    qreal currPercent = minPercent;
    for(int i = 0; i < 100; i++) {
        QPointF testPoint = mMappedEditPath.pointAtPercent(currPercent);
        qreal dist = distBetweenTwoPoints(testPoint, point);
        if(dist < smallestDist) {
            smallestDist = dist;
            nearestPoint = testPoint;
            nearestPercent = currPercent;
        }
        currPercent += percentStep;
    }
    return findPercentForPointEditPath(point, nearestPercent - percentStep,
                                       nearestPercent + percentStep);
}

PathPoint *VectorPath::findPointNearestToPercent(qreal percent,
                                                 qreal *foundAtPercent) {
    PathPoint *nearestPoint = mPoints.first();
    qreal nearestPointPercent = 100.;
    foreach(PathPoint *point, mPoints) {
        qreal pointPercent = findPercentForPoint(point->getAbsolutePos());
        if(qAbs(pointPercent - percent) < qAbs(nearestPointPercent - percent)) {
            nearestPointPercent = pointPercent;
            nearestPoint = point;
        }
    }
    *foundAtPercent = nearestPointPercent;
    return nearestPoint;
}

PathPoint *VectorPath::findPointNearestToPercentEditPath(qreal percent,
                                                 qreal *foundAtPercent) {
    PathPoint *nearestPoint = mPoints.first();
    qreal nearestPointPercent = 100.;
    foreach(PathPoint *point, mPoints) {
        qreal pointPercent = findPercentForPoint(point->getInfluenceAbsolutePos());
        if(qAbs(pointPercent - percent) < qAbs(nearestPointPercent - percent)) {
            nearestPointPercent = pointPercent;
            nearestPoint = point;
        }
    }
    *foundAtPercent = nearestPointPercent;
    return nearestPoint;
}

void VectorPath::attachToBoneFromSqlZId()
{
    BoundingBox::attachToBoneFromSqlZId();
    foreach(PathPoint *point, mPoints) {
        point->attachToBoneFromSqlZId();
    }
    mFillGradientPoints.attachToBoneFromSqlZId();
    mStrokeGradientPoints.attachToBoneFromSqlZId();
}

void VectorPath::updateAfterFrameChanged(int currentFrame)
{
    foreach(PathPoint *point, mPoints) {
        point->updateAfterFrameChanged(currentFrame);
    }
    mFillPaintSettings.setFrame(currentFrame);
    mStrokeSettings.setFrame(currentFrame);
    mPathAnimator.setFrame(currentFrame);
    BoundingBox::updateAfterFrameChanged(currentFrame);
}

PathPoint *VectorPath::createNewPointOnLineNear(QPointF absPos)
{
    qreal maxDist = 14.f;
    if(!mMappedPath.intersects(QRectF(absPos - QPointF(maxDist, maxDist),
                                     QSizeF(maxDist*2, maxDist*2))) ) {
        return NULL;
    }

    qreal nearestPercent = findPercentForPoint(absPos);
    QPointF nearestPtOnPath = mMappedPath.pointAtPercent(nearestPercent);
    if((nearestPtOnPath - absPos).manhattanLength() > maxDist ) {
        return NULL;
    }


    qreal nearestPtPercent;
    PathPoint *nearestPoint = findPointNearestToPercent(nearestPercent,
                                                        &nearestPtPercent);

    PathPoint *prevPoint;
    PathPoint *nextPoint;
    if(nearestPtPercent > nearestPercent) {
        prevPoint = nearestPoint->getPreviousPoint();
        nextPoint = nearestPoint;
    } else {
        nextPoint = nearestPoint->getNextPoint();
        prevPoint = nearestPoint;
    }

    qreal percent1 = findPercentForPoint(prevPoint->getAbsolutePos());
    qreal percent2 = findPercentForPoint(nextPoint->getAbsolutePos());
    if(nextPoint->isSeparatePathPoint() ) {
        percent2 += 1;
    }
    qreal minPercent = qMin(percent1, percent2);
    qreal maxPercent = qMax(percent1, percent2);
    qreal pressedT = (nearestPercent - minPercent) / (maxPercent - minPercent);
    if(pressedT > 0.0001 && pressedT < 0.9999) {
        startNewUndoRedoSet();

        QPointF prevPointEnd = prevPoint->getEndCtrlPtValue();
        QPointF nextPointStart = nextPoint->getStartCtrlPtValue();
        QPointF newPointPos;
        QPointF newPointStart;
        QPointF newPointEnd;
        Edge::getNewRelPosForKnotInsertionAtT(prevPoint->getRelativePos(),
                                              &prevPointEnd,
                                              &nextPointStart,
                                              nextPoint->getRelativePos(),
                                              &newPointPos,
                                              &newPointStart,
                                              &newPointEnd,
                                              pressedT);

        PathPoint *newPoint = new PathPoint(newPointPos, this);
        newPoint->setRelativePos(newPointPos, false);

        nextPoint->setPointAsPrevious(newPoint);
        prevPoint->setPointAsNext(newPoint);


        if(!prevPoint->isEndCtrlPtEnabled() && !nextPoint->isStartCtrlPtEnabled()) {
            newPoint->setStartCtrlPtEnabled(false);
            newPoint->setEndCtrlPtEnabled(false);
        } else {
            newPoint->setCtrlsMode(CtrlsMode::CTRLS_SMOOTH, false);
            newPoint->setStartCtrlPtEnabled(true);
            newPoint->moveStartCtrlPtToRelPos(newPointStart);
            newPoint->setEndCtrlPtEnabled(true);
            newPoint->moveEndCtrlPtToRelPos(newPointEnd);

            if(prevPoint->getCurrentCtrlsMode() == CtrlsMode::CTRLS_SYMMETRIC &&
                prevPoint->isEndCtrlPtEnabled() && prevPoint->isStartCtrlPtEnabled()) {
                prevPoint->setCtrlsMode(CtrlsMode::CTRLS_SMOOTH);
            }
            if(nextPoint->getCurrentCtrlsMode() == CtrlsMode::CTRLS_SYMMETRIC &&
                nextPoint->isEndCtrlPtEnabled() && nextPoint->isStartCtrlPtEnabled()) {
                nextPoint->setCtrlsMode(CtrlsMode::CTRLS_SMOOTH);
            }
            prevPoint->moveEndCtrlPtToRelPos(prevPointEnd);
            nextPoint->moveStartCtrlPtToRelPos(nextPointStart);
        }

        appendToPointsList(newPoint);

        finishUndoRedoSet();
        return newPoint;
    }
    return NULL;
}

Edge *VectorPath::getEdgeFromMappedPath(QPointF absPos) {
    qreal maxDist = 14.f;
    if(!mMappedPath.intersects(QRectF(absPos - QPointF(maxDist, maxDist),
                                     QSizeF(maxDist*2, maxDist*2))) ) {
        return NULL;
    }

    qreal nearestPercent = findPercentForPoint(absPos);
    QPointF nearestPtOnPath = mMappedPath.pointAtPercent(nearestPercent);

    if((nearestPtOnPath - absPos).manhattanLength() > maxDist ) {
        return NULL;
    }

    qreal nearestPtPercent;
    PathPoint *nearestPoint = findPointNearestToPercent(nearestPercent,
                                                        &nearestPtPercent);

    PathPoint *prevPoint;
    PathPoint *nextPoint;

    PathPoint *nextPointS;
    PathPoint *prevPointS;

    if(nearestPtPercent > nearestPercent) {
        prevPoint = nearestPoint->getPreviousPoint();
        nextPoint = nearestPoint;
    } else {
        nextPoint = nearestPoint->getNextPoint();
        prevPoint = nearestPoint;
    }

    nextPointS = nextPoint;
    prevPointS = prevPoint;

    while(!nextPoint->hasFullInfluence() ) {
        nextPoint = nextPoint->getNextPoint();
        if(nextPoint == nextPointS || nextPoint == NULL) return NULL;
    }

    while(!prevPoint->hasFullInfluence() ) {
        prevPoint = prevPoint->getPreviousPoint();
        if(prevPoint == prevPointS || prevPoint == NULL) return NULL;
    }

    if(!nextPoint->isStartCtrlPtEnabled() ) {
        nextPoint->setStartCtrlPtEnabled(true);
    }
    if(!prevPoint->isEndCtrlPtEnabled() ) {
        prevPoint->setEndCtrlPtEnabled(true);
    }

    qreal percent1 = findPercentForPoint(prevPoint->getAbsolutePos());
    qreal percent2 = findPercentForPoint(nextPoint->getAbsolutePos());
    if(percent2 < percent1 ) {
        percent2 += 1.;
        if(nearestPercent < percent1) {
            nearestPercent += 1.;
        }
    }
    qreal minPercent = qMin(percent1, percent2);
    qreal maxPercent = qMax(percent1, percent2);
    qreal pressedT = (nearestPercent - minPercent) / (maxPercent - minPercent);
    if(pressedT > 0.0001 && pressedT < 0.9999) {
        return new Edge(prevPoint, nextPoint, pressedT);
    } else {
        return NULL;
    }
}

Edge *VectorPath::getEdgeFromMappedEditPath(QPointF absPos)
{
    qreal maxDist = 14.f;
    if(!mMappedEditPath.intersects(QRectF(absPos - QPointF(maxDist, maxDist),
                                     QSizeF(maxDist*2, maxDist*2))) ) {
        return NULL;
    }

    qreal nearestPercent = findPercentForPointEditPath(absPos);
    QPointF nearestPtOnPath = mMappedEditPath.pointAtPercent(nearestPercent);

    if((nearestPtOnPath - absPos).manhattanLength() > maxDist ) {
        return NULL;
    }

    qreal nearestPtPercent;
    PathPoint *nearestPoint = findPointNearestToPercentEditPath(nearestPercent,
                                                        &nearestPtPercent);

    PathPoint *prevPoint;
    PathPoint *nextPoint;

    if(nearestPtPercent > nearestPercent) {
        prevPoint = nearestPoint->getPreviousPoint();
        nextPoint = nearestPoint;
    } else {
        nextPoint = nearestPoint->getNextPoint();
        prevPoint = nearestPoint;
    }

    if(!nextPoint->isStartCtrlPtEnabled() ) {
        nextPoint->setStartCtrlPtEnabled(true);
    }
    if(!prevPoint->isEndCtrlPtEnabled() ) {
        prevPoint->setEndCtrlPtEnabled(true);
    }

    qreal percent1 = findPercentForPointEditPath(prevPoint->getAbsolutePos());
    qreal percent2 = findPercentForPointEditPath(nextPoint->getAbsolutePos());
    if(percent2 < percent1 ) {
        percent2 += 1.;
        if(nearestPercent < percent1) {
            nearestPercent += 1.;
        }
    }
    qreal minPercent = qMin(percent1, percent2);
    qreal maxPercent = qMax(percent1, percent2);
    qreal pressedT = (nearestPercent - minPercent) / (maxPercent - minPercent);
    if(pressedT > 0.0001 && pressedT < 0.9999) {
        return new Edge(prevPoint, nextPoint, pressedT);
    } else {
        return NULL;
    }
}

Edge *VectorPath::getEgde(QPointF absPos) {
    Edge *edgeT = NULL;
    if(mInfluenceEnabled) {
        edgeT = getEdgeFromMappedEditPath(absPos);
    }
    if(edgeT == NULL) {
        edgeT = getEdgeFromMappedPath(absPos);
    }
    return edgeT;
}

void VectorPath::centerPivotPosition() {
    if(!mPivotChanged) {
        QPointF posSum = QPointF(0.f, 0.f);
        int count = mPoints.length();
        if(count == 0) return;
        foreach(PathPoint *point, mPoints) {
            posSum += point->getRelativePos();
        }
        mTransformAnimator.setPivot(posSum/count);
    }
}

const PaintSettings *VectorPath::getFillSettings()
{
    return &mFillPaintSettings;
}

const StrokeSettings *VectorPath::getStrokeSettings()
{
    return &mStrokeSettings;
}

/*void VectorPath::setStrokeSettings(StrokeSettings strokeSettings, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        addUndoRedo(new StrokeSettingsChangedUndoRedo(mStrokeSettings,
                                                      strokeSettings,
                                                      this) );
    }
    bool wasGradient = mStrokeSettings.paintType == GRADIENTPAINT;
    if(wasGradient) {
        mStrokeSettings.gradient->removePath(this);
    }
    mStrokeSettings = strokeSettings;
    updateOutlinePath();
    if(mStrokeSettings.paintType == GRADIENTPAINT && !wasGradient) {
        mStrokeGradientPoints.setPositions(getBoundingRect().topLeft(),
                     getBoundingRect().bottomRight(), saveUndoRedo);
    }
    
}

void VectorPath::setFillSettings(PaintSettings fillSettings, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        addUndoRedo(new FillSettingsChangedUndoRedo(mFillPaintSettings,
                                                       fillSettings,
                                                     this) );
    }
    bool wasGradient = mFillPaintSettings.paintType == GRADIENTPAINT;
    if(wasGradient) {
        mFillPaintSettings.gradient->removePath(this);
    }
    mFillPaintSettings = fillSettings;
    updateDrawGradients();
    if(mFillPaintSettings.paintType == GRADIENTPAINT && !wasGradient) {
        mFillGradientPoints.setPositions(getBoundingRect().topLeft(),
                     getBoundingRect().bottomRight(), saveUndoRedo);
    }
    
}*/

void VectorPath::updatePath()
{
    mPath = QPainterPath();
    mPath.setFillRule(Qt::WindingFill);
    foreach(PathPoint *point, mPoints) {
        point->clearInfluenceAdjustedPointValues();
    }

    if(mInfluenceEnabled) {
        mEditPath = QPainterPath();

        bool moreNeeded = true;
        while(moreNeeded) {
            moreNeeded = false;
            foreach(PathPoint *point, mPoints) {
                if(point->updateInfluenceAdjustedPointValues() ) {
                    moreNeeded = true;
                }
            }
        }

        foreach(PathPoint *point, mPoints) {
            point->finishInfluenceAdjusted();
        }
    }

    foreach (PathPoint *firstPointInPath, mSeparatePaths) {
        PathPoint *point = firstPointInPath;
        PathPointValues lastPointValues = point->getInfluenceAdjustedPointValues();
        mPath.moveTo(lastPointValues.pointRelPos);
        while(true) {
            point = point->getNextPoint();
            if(point == NULL) break;
            PathPointValues pointValues = point->getInfluenceAdjustedPointValues();

            mPath.cubicTo(lastPointValues.endRelPos,
                          pointValues.startRelPos,
                          pointValues.pointRelPos);

            lastPointValues = pointValues;

            if(point == firstPointInPath) break;
        }

        if(mInfluenceEnabled) {
            point = firstPointInPath;
            lastPointValues = point->getPointValues();
            mEditPath.moveTo(lastPointValues.pointRelPos);
            while(true) {
                point = point->getNextPoint();
                if(point == NULL) break;
                PathPointValues pointValues = point->getPointValues();

                mEditPath.cubicTo(lastPointValues.endRelPos,
                              pointValues.startRelPos,
                              pointValues.pointRelPos);

                lastPointValues = pointValues;

                if(point == firstPointInPath) break;
            }
        }
    }

    updateMappedPath();
}

//void VectorPath::updatePath()
//{
//    mPath = QPainterPath();
//    mPath.setFillRule(Qt::WindingFill);
//    foreach (PathPoint *firstPointInPath, mSeparatePaths) {
//        PathPoint *point = NULL;
//        PathPoint *lastPoint = firstPointInPath;
//        mPath.moveTo(firstPointInPath->getRelativePos());
//        while(true) {
//            point = lastPoint->getNextPoint();
//            if(point == NULL) {
//                break;
//            }
//            qreal pointInf = point->getCurrentInfluence();
//            qreal pointInfT = point->getCurrentInfluenceT();
//            QPointF pointPos = point->getRelativePos();
//            pointPos = pointPos*pointInf +
//                    (1. - pointInf)*Edge::getRelPosBetweenPointsAtT(pointInfT,
//                                                                    lastPoint,
//                                                                    point->getNextPoint() );
//            mPath.cubicTo(lastPoint->getEndCtrlPtValue(),
//                          point->getStartCtrlPtValue(),
//                          pointPos);
//            if(point == firstPointInPath) {
//                break;
//            }
//            lastPoint = point;
//        }
//    }

//    updateMappedPath();
//}

PathPoint *VectorPath::addPointRelPos(QPointF relPos,
                                      QPointF startRelPos, QPointF endRelPos,
                                      PathPoint *toPoint) {
    PathPoint *newPoint = addPointRelPos(relPos, toPoint);
    newPoint->moveStartCtrlPtToRelPos(startRelPos);
    newPoint->moveEndCtrlPtToRelPos(endRelPos);
    return newPoint;
}

void VectorPath::schedulePathUpdate()
{
    if(mPathUpdateNeeded) {
        return;
    }
    addUpdateScheduler(new PathUpdateScheduler(this));
    mPathUpdateNeeded = true;
    mMappedPathUpdateNeeded = false;
    mOutlinePathUpdateNeeded = false;
}

void VectorPath::updatePathIfNeeded()
{
    if(mPathUpdateNeeded) {
        updatePath();
        if(!mAnimatorsCollection.hasKeys() ) centerPivotPosition();
        mPathUpdateNeeded = false;
        mMappedPathUpdateNeeded = false;
        mOutlinePathUpdateNeeded = false;
    }
}

void VectorPath::scheduleOutlinePathUpdate()
{
    if(mOutlinePathUpdateNeeded) {
        return;
    }
    addUpdateScheduler(new OutlineUpdateScheduler(this));
    mOutlinePathUpdateNeeded = true;
}

void VectorPath::updateOutlinePathIfNeeded() {
    if(mOutlinePathUpdateNeeded) {
        updateOutlinePath();
        mOutlinePathUpdateNeeded = false;
    }
}

void VectorPath::updateMappedPathIfNeeded()
{
    if(mMappedPathUpdateNeeded) {
        if(mParent != NULL) {
            updateMappedPath();
        }
        mMappedPathUpdateNeeded = false;
    }
}

void VectorPath::scheduleMappedPathUpdate()
{
    if(mMappedPathUpdateNeeded || mPathUpdateNeeded || mParent == NULL) {
        return;
    }
    addUpdateScheduler(new MappedPathUpdateScheduler(this));
    mMappedPathUpdateNeeded = true;
    
}

void VectorPath::updateAfterCombinedTransformationChanged()
{
    scheduleMappedPathUpdate();
}

void VectorPath::updateOutlinePath() {
    mStrokeSettings.setStrokerSettings(&mPathStroker);
    mMappedOutlinePath = mCombinedTransformMatrix.map(mPathStroker.createStroke(mPath));
    updateWholePath();
}

void VectorPath::updateWholePath() {
    mMappedWhole = QPainterPath();
    if(mStrokeSettings.getPaintType() != NOPAINT) {
        mMappedWhole += mMappedOutlinePath;
    }
    if(mFillPaintSettings.getPaintType() != NOPAINT ||
            mStrokeSettings.getPaintType() == NOPAINT) {
        mMappedWhole += mMappedPath;
    }
}

void VectorPath::setRenderCombinedTransform() {
    BoundingBox::setRenderCombinedTransform();
    updateMappedPath();
}

void VectorPath::updatePathPointIds()
{
    int pointId = 1;
    foreach(PathPoint *point, mSeparatePaths) {
        PathPoint *nextPoint = point;
        while(true) {
            nextPoint->setPointId(pointId);
            pointId++;
            nextPoint = nextPoint->getNextPoint();
            if(nextPoint == NULL || nextPoint == point) break;
        }
    }
}

void VectorPath::showContextMenu(QPoint globalPos) {
    QMenu menu(mMainWindow);

    QAction  *infAction = new QAction("Points influence");
    infAction->setCheckable(true);
    infAction->setChecked(mInfluenceEnabled);

    menu.addAction(infAction);

    menu.addAction("Delete");
    QAction *selected_action = menu.exec(globalPos);
    if(selected_action != NULL)
    {
        if(selected_action->text() == "Delete")
        {

        } else if(selected_action == infAction) {
            if(mInfluenceEnabled) {
                disableInfluence();
            } else {
                enableInfluence();
            }
        }
    } else {

    }
}

void VectorPath::updateMappedPath()
{
    mMappedPath = mCombinedTransformMatrix.map(mPath);
    if(mInfluenceEnabled) {
        mMappedEditPath = mCombinedTransformMatrix.map(mEditPath);
    }
    updateOutlinePath();
    updateDrawGradients();
}

void VectorPath::updateDrawGradients()
{
    if(mFillPaintSettings.getPaintType() == GRADIENTPAINT) {
        Gradient *gradient = mFillPaintSettings.getGradient();
        if(!gradient->isInPaths(this)) {
            gradient->addPath(this);
        }
        mFillGradientPoints.setColors(gradient->getFirstQGradientStopQColor(),
                                      gradient->getLastQGradientStopQColor());
        if(!mFillGradientPoints.enabled) {
            mFillGradientPoints.enable();
        }

        mDrawFillGradient.setStops(gradient->getQGradientStops());
        mDrawFillGradient.setStart(mFillGradientPoints.getStartPoint() );
        mDrawFillGradient.setFinalStop(mFillGradientPoints.getEndPoint() );

    } else if(mFillGradientPoints.enabled) {
        mFillGradientPoints.disable();
    }
    if(mStrokeSettings.getPaintType() == GRADIENTPAINT) {
        Gradient *gradient = mStrokeSettings.getGradient();
        if(!gradient->isInPaths(this)) {
            gradient->addPath(this);
        }
        mStrokeGradientPoints.setColors(gradient->getFirstQGradientStopQColor(),
                                      gradient->getLastQGradientStopQColor() );

        if(!mStrokeGradientPoints.enabled) {
            mStrokeGradientPoints.enable();
        }
        mDrawStrokeGradient.setStops(gradient->getQGradientStops());
        mDrawStrokeGradient.setStart(mStrokeGradientPoints.getStartPoint() );
        mDrawStrokeGradient.setFinalStop(mStrokeGradientPoints.getEndPoint() );
    } else if(mStrokeGradientPoints.enabled) {
        mStrokeGradientPoints.disable();
    }
}

QRectF VectorPath::getBoundingRect()
{
    return mMappedWhole.boundingRect();
}

void VectorPath::draw(QPainter *p)
{
    if(mVisible) {
        p->save();

        p->setOpacity(p->opacity()*mTransformAnimator.getOpacity()*0.01 );
        p->setPen(Qt::NoPen);
        if(mFillPaintSettings.getPaintType() == GRADIENTPAINT) {
            p->setBrush(mDrawFillGradient);
        } else if(mFillPaintSettings.getPaintType() == FLATPAINT) {
            p->setBrush(mFillPaintSettings.getCurrentColor().qcol);
        } else{
            p->setBrush(Qt::NoBrush);
        }
        p->drawPath(mMappedPath);
        if(mStrokeSettings.getPaintType() == GRADIENTPAINT) {
            p->setBrush(mDrawStrokeGradient);
        } else if(mStrokeSettings.getPaintType() == FLATPAINT) {
            p->setBrush(mStrokeSettings.getCurrentColor().qcol);
        } else{
            p->setBrush(Qt::NoBrush);
        }
        p->drawPath(mMappedOutlinePath);

        p->restore();
    }
}

void VectorPath::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    if(mVisible) {
        p->save();
        drawBoundingRect(p);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            if(mInfluenceEnabled) {
                p->setBrush(Qt::NoBrush);
                p->setPen(QPen(Qt::blue, 1., Qt::DashLine) );
                p->drawPath(mMappedEditPath);
            }
            p->setPen(QPen(QColor(0, 0, 0, 255), 1.5));
            PathPoint *point;
            foreachInverted(point, mPoints) {
                point->draw(p, currentCanvasMode);
            }
            mFillGradientPoints.drawGradientPoints(p);
            mStrokeGradientPoints.drawGradientPoints(p);
        } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
            p->setPen(QPen(QColor(0, 0, 0, 255), 1.5));
            PathPoint *point;
            foreachInverted(point, mPoints) {
                if(point->isEndPoint() || point->isSelected()) {
                    point->draw(p, currentCanvasMode);
                }
            }
        }
        p->restore();
    }
}

MovablePoint *VectorPath::getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode)
{
    MovablePoint *pointToReturn = NULL;
    if(currentCanvasMode == MOVE_POINT) {
        pointToReturn = mStrokeGradientPoints.getPointAt(absPtPos);
        if(pointToReturn == NULL) {
            pointToReturn = mFillGradientPoints.getPointAt(absPtPos);
        }
    }
    if(pointToReturn == NULL) {
        foreach(PathPoint *point, mPoints) {
            pointToReturn = point->getPointAtAbsPos(absPtPos, currentCanvasMode);
            if(pointToReturn != NULL) {
                break;
            }
        }
    }
    return pointToReturn;
}

void VectorPath::selectAndAddContainedPointsToList(QRectF absRect,
                                                   QList<MovablePoint *> *list)
{
    foreach(PathPoint *point, mPoints) {
        point->rectPointsSelection(absRect, list);
    }
}

void VectorPath::addPointToSeparatePaths(PathPoint *pointToAdd,
                                         bool saveUndoRedo) {
    mSeparatePaths.append(pointToAdd);
    pointToAdd->setSeparatePathPoint(true);

    if(saveUndoRedo) {
        AddPointToSeparatePathsUndoRedo *undoRedo = new AddPointToSeparatePathsUndoRedo(this, pointToAdd);
        addUndoRedo(undoRedo);
    }
    schedulePathUpdate();

    updatePathPointIds();
}

void VectorPath::removePointFromSeparatePaths(PathPoint *pointToRemove,
                                              bool saveUndoRedo) {
    pointToRemove->setSeparatePathPoint(false);
    mSeparatePaths.removeOne(pointToRemove);

    if(saveUndoRedo) {
        RemovePointFromSeparatePathsUndoRedo *undoRedo = new RemovePointFromSeparatePathsUndoRedo(this, pointToRemove);
        addUndoRedo(undoRedo);
    }
    schedulePathUpdate();

    updatePathPointIds();
}

PathPoint *VectorPath::addPoint(PathPoint *pointToAdd, PathPoint *toPoint)
{
    startNewUndoRedoSet();

    if(toPoint == NULL) {
        addPointToSeparatePaths(pointToAdd);
    } else {
        if(!toPoint->hasNextPoint()) {
            toPoint->setPointAsNext(pointToAdd);
        } else if(!toPoint->hasPreviousPoint() ) {
            replaceSeparatePathPoint(toPoint, pointToAdd);
            toPoint->setPointAsPrevious(pointToAdd);
        }
    }

    appendToPointsList(pointToAdd);
    finishUndoRedoSet();

    return pointToAdd;
}

PathPoint* VectorPath::addPointAbsPos(QPointF absPtPos, PathPoint *toPoint)
{
    PathPoint *newPoint = new PathPoint(absPtPos, this);

    return addPoint(newPoint, toPoint);
}

PathPoint *VectorPath::addPointRelPos(QPointF relPtPos, PathPoint *toPoint)
{
    PathPoint *newPoint = new PathPoint(QPointF(0.f, 0.f), this);
    newPoint->setRelativePos(relPtPos);

    return addPoint(newPoint, toPoint);
}

void VectorPath::appendToPointsList(PathPoint *point, bool saveUndoRedo) {
    mPoints.append(point);
    mPathAnimator.addChildAnimator(point->getPathPointAnimatorsPtr());
    point->show();
    if(saveUndoRedo) {
        AppendToPointsListUndoRedo *undoRedo = new AppendToPointsListUndoRedo(point, this);
        addUndoRedo(undoRedo);
    }
    point->incNumberPointers();

    updatePathPointIds();
}

void VectorPath::removeFromPointsList(PathPoint *point, bool saveUndoRedo) {
    mPoints.removeOne(point);
    mPathAnimator.removeChildAnimator(point->getPathPointAnimatorsPtr());
    point->hide();
    point->deselect();
    if(saveUndoRedo) {
        startNewUndoRedoSet();
        RemoveFromPointsListUndoRedo *undoRedo = new RemoveFromPointsListUndoRedo(point, this);
        addUndoRedo(undoRedo);
        if(mPoints.count() < 2) {
            mParent->removeChild(this);
        }
        finishUndoRedoSet();
    }
    point->decNumberPointers();

    updatePathPointIds();
}

void VectorPath::removePoint(PathPoint *point) {
    startNewUndoRedoSet();
    PathPoint *prevPoint = point->getPreviousPoint();
    PathPoint *nextPoint = point->getNextPoint();

    if(prevPoint != NULL) {
        if(point->isSeparatePathPoint()) {
            replaceSeparatePathPoint(point, prevPoint);
        }
        prevPoint->setPointAsNext(nextPoint);
    } else if (nextPoint != NULL){
        replaceSeparatePathPoint(point, nextPoint);
        nextPoint->setPointAsPrevious(NULL);
    } else {
        removePointFromSeparatePaths(point);
    }
    removeFromPointsList(point);

    finishUndoRedoSet();
}

bool VectorPath::pointInsidePath(QPointF point)
{
    return mMappedWhole.contains(point);
}

void VectorPath::replaceSeparatePathPoint(PathPoint *pointBeingReplaced,
                                          PathPoint *newPoint) {
    startNewUndoRedoSet();
    removePointFromSeparatePaths(pointBeingReplaced);
    addPointToSeparatePaths(newPoint);
    finishUndoRedoSet();
}
#include <QSqlError>
int VectorPath::saveToSql(int parentId)
{
    QSqlQuery query;
    int boundingBoxId = BoundingBox::saveToSql(parentId);
    int fillStartPt = mFillGradientPoints.startPoint->saveToSql();
    int fillEndPt = mFillGradientPoints.endPoint->saveToSql();
    int strokeStartPt = mStrokeGradientPoints.startPoint->saveToSql();
    int strokeEndPt = mStrokeGradientPoints.endPoint->saveToSql();

    int fillSettingsId = mFillPaintSettings.saveToSql();
    int strokeSettingsId = mStrokeSettings.saveToSql();
    if(!query.exec(
            QString(
            "INSERT INTO vectorpath (fillgradientstartid, fillgradientendid, "
            "strokegradientstartid, strokegradientendid, "
            "boundingboxid, fillsettingsid, strokesettingsid) "
            "VALUES (%1, %2, %3, %4, %5, %6, %7)").
            arg(fillStartPt).
            arg(fillEndPt).
            arg(strokeStartPt).
            arg(strokeEndPt).
            arg(boundingBoxId).
            arg(fillSettingsId).
            arg(strokeSettingsId) ) ) {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }
    int vectorPathId = query.lastInsertId().toInt();
    foreach(PathPoint *point, mSeparatePaths) {
        point->saveToSql(vectorPathId);
    }

    return boundingBoxId;
}

void VectorPath::startAllPointsTransform()
{
    foreach(PathPoint *point, mPoints) {
        point->startTransform();
    }
}

void VectorPath::finishAllPointsTransform()
{
    foreach(PathPoint *point, mPoints) {
        point->finishTransform();
    }
}

GradientPoints::GradientPoints() : ComplexAnimator()
{
    setName("gradient points");
}

void GradientPoints::initialize(VectorPath *parentT,
                                QPointF startPt, QPointF endPt)
{
    parent = parentT;
    startPoint = new GradientPoint(startPt, parent);
    startPoint->incNumberPointers();
    addChildAnimator(startPoint->getRelativePosAnimatorPtr() );
    startPoint->getRelativePosAnimatorPtr()->setName("point1");
    endPoint = new GradientPoint(endPt, parent);
    endPoint->getRelativePosAnimatorPtr()->setName("point2");
    endPoint->incNumberPointers();
    addChildAnimator(endPoint->getRelativePosAnimatorPtr() );
    enabled = false;
}

void GradientPoints::initialize(VectorPath *parentT,
                                int fillGradientStartId, int fillGradientEndId)
{
    parent = parentT;
    startPoint = new GradientPoint(fillGradientStartId, parent);
    startPoint->incNumberPointers();
    addChildAnimator(startPoint->getRelativePosAnimatorPtr() );
    startPoint->getRelativePosAnimatorPtr()->setName("point1");
    endPoint = new GradientPoint(fillGradientEndId, parent);
    endPoint->incNumberPointers();
    endPoint->getRelativePosAnimatorPtr()->setName("point2");
    addChildAnimator(endPoint->getRelativePosAnimatorPtr() );
    enabled = false;
}

void GradientPoints::clearAll()
{
    delete startPoint;
    delete endPoint;
}

void GradientPoints::enable()
{
    if(enabled) {
        return;
    }
    enabled = true;
}

void GradientPoints::setPositions(QPointF startPos, QPointF endPos, bool saveUndoRedo) {
    startPoint->setAbsolutePos(startPos, saveUndoRedo);
    endPoint->setAbsolutePos(endPos, saveUndoRedo);
}

void GradientPoints::disable()
{
    enabled = false;
}

void GradientPoints::drawGradientPoints(QPainter *p)
{
    if(enabled) {
       p->drawLine(startPoint->getAbsolutePos(), endPoint->getAbsolutePos());
       startPoint->draw(p);
       endPoint->draw(p);
    }
}

MovablePoint *GradientPoints::getPointAt(QPointF absPos)
{
    if(enabled) {
        if(startPoint->isPointAt(absPos) ) {
            return startPoint;
        } else if (endPoint->isPointAt(absPos) ){
            return endPoint;
        }
    }
    return NULL;
}

QPointF GradientPoints::getStartPoint()
{
    return startPoint->getAbsolutePos();
}

QPointF GradientPoints::getEndPoint()
{
    return endPoint->getAbsolutePos();
}

void GradientPoints::setColors(QColor startColor, QColor endColor)
{
    startPoint->setColor(startColor);
    endPoint->setColor(endColor);
}

void GradientPoints::attachToBoneFromSqlZId()
{
    startPoint->attachToBoneFromSqlZId();
    endPoint->attachToBoneFromSqlZId();
}
