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
    mFillGradientPoints.initialize(this);
    mStrokeGradientPoints.initialize(this);
}

VectorPath::VectorPath(int boundingBoxId,
                        BoxesGroup *parent) :
    BoundingBox(boundingBoxId,
                parent, TYPE_VECTOR_PATH) {
    QSqlQuery query;
    QString queryStr = "SELECT * FROM vectorpath WHERE boundingboxid = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr) ) {
        query.next();
        int idId = query.record().indexOf("id");
        int idfillgradientstartx = query.record().indexOf("fillgradientstartx");
        int idfillgradientstarty = query.record().indexOf("fillgradientstarty");
        int idfillgradientendx = query.record().indexOf("fillgradientendx");
        int idfillgradientendy = query.record().indexOf("fillgradientendy");
        int idstrokegradientstartx = query.record().indexOf("strokegradientstartx");
        int idstrokegradientstarty = query.record().indexOf("strokegradientstarty");
        int idstrokegradientendx = query.record().indexOf("strokegradientendx");
        int idstrokegradientendy = query.record().indexOf("strokegradientendy");
        int idfillsettingsid = query.record().indexOf("fillsettingsid");
        int idstrokesettingsid = query.record().indexOf("strokesettingsid");


        int vectorPathId = query.value(idId).toInt();
        qreal fillGradientStartX = query.value(idfillgradientstartx).toReal();
        qreal fillGradientStartY = query.value(idfillgradientstarty).toReal();
        qreal fillGradientEndX = query.value(idfillgradientendx).toReal();
        qreal fillGradientEndY = query.value(idfillgradientendy).toReal();
        qreal strokeGradientStartX = query.value(idstrokegradientstartx).toReal();
        qreal strokeGradientStartY = query.value(idstrokegradientstarty).toReal();
        qreal strokeGradientEndX = query.value(idstrokegradientendx).toReal();
        qreal strokeGradientEndY = query.value(idstrokegradientendy).toReal();
        int fillSettingsId = query.value(idfillsettingsid).toInt();
        int strokeSettingsId = query.value(idstrokesettingsid).toInt();

        loadPointsFromSql(vectorPathId);

        mFillGradientPoints.initialize(this,
                               QPointF(fillGradientStartX, fillGradientStartY),
                               QPointF(fillGradientEndX, fillGradientEndY));
        mStrokeGradientPoints.initialize(this,
                               QPointF(strokeGradientStartX, strokeGradientStartY),
                               QPointF(strokeGradientEndX, strokeGradientEndY));

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
    QString queryStr = QString("SELECT * FROM pathpoint WHERE vectorpathid = %1 "
                               "ORDER BY id ASC").arg(vectorPathId);
    if(query.exec(queryStr) ) {
        int idisfirst = query.record().indexOf("isfirst");
        int idisendpoint = query.record().indexOf("isendpoint");
        int idxrelpos = query.record().indexOf("xrelpos");
        int idyrelpos = query.record().indexOf("yrelpos");
        int idstartctrlptrelx = query.record().indexOf("startctrlptrelx");
        int idstartctrlptrely = query.record().indexOf("startctrlptrely");
        int idendctrlptrelx = query.record().indexOf("endctrlptrelx");
        int idendctrlptrely = query.record().indexOf("endctrlptrely");
        int idbonez = query.record().indexOf("bonezid");
        PathPoint *firstPoint = NULL;
        PathPoint *lastPoint = NULL;
        while(query.next()) {
            bool isfirst = query.value(idisfirst).toBool();
            bool isendpoint = query.value(idisendpoint).toBool();
            qreal xrelpos = query.value(idxrelpos).toReal();
            qreal yrelpos = query.value(idyrelpos).toReal();
            qreal startctrlptrelx = query.value(idstartctrlptrelx).toReal();
            qreal startctrlptrely = query.value(idstartctrlptrely).toReal();
            qreal endctrlptrelx = query.value(idendctrlptrelx).toReal();
            qreal endctrlptrely = query.value(idendctrlptrely).toReal();
            int bonez = query.value(idbonez).toInt();

            PathPoint *newPoint = new PathPoint(xrelpos, yrelpos,
                          startctrlptrelx, startctrlptrely,
                          endctrlptrelx, endctrlptrely, isfirst, bonez, this);
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

PathPoint *VectorPath::findPointNearestToPercent(qreal percent,
                                                 qreal *foundAtPercent) {
    PathPoint *nearestPoint = mPoints.first();
    qreal nearestPointPercent = 100.f;
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

void VectorPath::attachToBoneFromSqlZId()
{
    BoundingBox::attachToBoneFromSqlZId();
    foreach(PathPoint *point, mPoints) {
        point->attachToBoneFromSqlZId();
    }
    mFillGradientPoints.attachToBoneFromSqlZId();
    mStrokeGradientPoints.attachToBoneFromSqlZId();
}

PathPoint *VectorPath::createNewPointOnLineNear(QPointF absPos)
{
    qreal maxDist = 14.f;
    if(!mMappedPath.intersects(QRectF(absPos - QPointF(maxDist, maxDist),
                                     QSizeF(maxDist*2, maxDist*2))) ) {
        return NULL;
    }
    startNewUndoRedoSet();

    qreal nearestPercent = findPercentForPoint(absPos);
    QPointF nearestPtOnPath = mMappedPath.pointAtPercent(nearestPercent);
    qreal nearestPtPercent;
    PathPoint *nearestPoint = findPointNearestToPercent(nearestPercent,
                                                        &nearestPtPercent);
    PathPoint *newPoint = new PathPoint(nearestPtOnPath, this);
    if(nearestPtPercent > nearestPercent) {
        PathPoint *prevPoint = nearestPoint->getPreviousPoint();
        nearestPoint->setPointAsPrevious(newPoint);
        prevPoint->setPointAsNext(newPoint);
    } else {
        PathPoint *nextPoint = nearestPoint->getNextPoint();
        nearestPoint->setPointAsNext(newPoint);
        nextPoint->setPointAsPrevious(newPoint);
    }
    appendToPointsList(newPoint);

    finishUndoRedoSet();
    return newPoint;
}

void VectorPath::updatePivotPosition() {
    if(!mPivotChanged) {
        QPointF posSum = QPointF(0.f, 0.f);
        int count = mPoints.length();
        foreach(PathPoint *point, mPoints) {
            posSum += point->getRelativePos();
        }
        mRelRotPivotPos = posSum/count;
    }
}

PaintSettings VectorPath::getFillSettings()
{
    return mFillPaintSettings;
}

StrokeSettings VectorPath::getStrokeSettings()
{
    return mStrokeSettings;
}

void VectorPath::setStrokeSettings(StrokeSettings strokeSettings, bool saveUndoRedo)
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
    scheduleRepaint();
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
    scheduleRepaint();
}

void VectorPath::updatePath()
{
    mPath = QPainterPath();
    mPath.setFillRule(Qt::WindingFill);
    foreach (PathPoint *firstPointInPath, mSeparatePaths) {
        PathPoint *point = NULL;
        PathPoint *lastPoint = firstPointInPath;
        mPath.moveTo(firstPointInPath->getRelativePos());
        while(true) {
            point = lastPoint->getNextPoint();
            if(point == NULL) {
                break;
            }
            QPointF pointPos = point->getRelativePos();
            mPath.cubicTo(lastPoint->getEndCtrlPtValue(),
                          point->getStartCtrlPtValue(),
                          pointPos);
            if(point == firstPointInPath) {
                break;
            }
            lastPoint = point;
        }
    }

    updateMappedPath();
}

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
    scheduleRepaint();
}

void VectorPath::updatePathIfNeeded()
{
    if(mPathUpdateNeeded) {
        mPathUpdateNeeded = false;
        mMappedPathUpdateNeeded = false;
        updatePath();
        updatePivotPosition();
    }
}

void VectorPath::updateMappedPathIfNeeded()
{
    if(mMappedPathUpdateNeeded && mParent != NULL) {
        mMappedPathUpdateNeeded = false;
        updateMappedPath();
    }
}

void VectorPath::scheduleMappedPathUpdate()
{
    if(mMappedPathUpdateNeeded || mPathUpdateNeeded || mParent == NULL) {
        return;
    }
    addUpdateScheduler(new MappedPathUpdateScheduler(this));
    mMappedPathUpdateNeeded = true;
    scheduleRepaint();
}

void VectorPath::updateAfterCombinedTransformationChanged()
{
    scheduleMappedPathUpdate();
}

void VectorPath::updateOutlinePath() {
    mStrokeSettings.setStrokerSettings(&mPathStroker);
    mOutlinePath = mCombinedTransformMatrix.map(mPathStroker.createStroke(mPath));
    updateWholePath();
}

void VectorPath::updateWholePath() {
    mMappedWhole = QPainterPath();
    if(mStrokeSettings.paintType != NOPAINT) {
        mMappedWhole += mOutlinePath;
    }
    if(mFillPaintSettings.paintType != NOPAINT ||
            mStrokeSettings.paintType == NOPAINT) {
        mMappedWhole += mMappedPath;
    }
}

void VectorPath::updateMappedPath()
{
    mMappedPath = mCombinedTransformMatrix.map(mPath);
    updateOutlinePath();
    updateDrawGradients();
}

void VectorPath::updateDrawGradients()
{
    if(mFillPaintSettings.paintType == GRADIENTPAINT) {
        Gradient *gradient = mFillPaintSettings.gradient;
        if(!gradient->isInPaths(this)) {
            gradient->addPath(this);
        }
        mFillGradientPoints.setColors(gradient->qgradientStops.first().second,
                                      gradient->qgradientStops.last().second);
        mFillGradientPoints.enable();
        mDrawFillGradient.setStops(gradient->qgradientStops);
        mDrawFillGradient.setStart(mFillGradientPoints.getStartPoint() );
        mDrawFillGradient.setFinalStop(mFillGradientPoints.getEndPoint() );
    } else {
        mFillGradientPoints.disable();
    }
    if(mStrokeSettings.paintType == GRADIENTPAINT) {
        Gradient *gradient = mStrokeSettings.gradient;
        if(!gradient->isInPaths(this)) {
            gradient->addPath(this);
        }
        mStrokeGradientPoints.setColors(gradient->qgradientStops.first().second,
                                      gradient->qgradientStops.last().second);

        mStrokeGradientPoints.enable();
        mDrawStrokeGradient.setStops(gradient->qgradientStops);
        mDrawStrokeGradient.setStart(mStrokeGradientPoints.getStartPoint() );
        mDrawStrokeGradient.setFinalStop(mStrokeGradientPoints.getEndPoint() );
    } else {
        mStrokeGradientPoints.disable();
    }
}

void VectorPath::startStrokeTransform()
{
    mSavedStrokeSettings = mStrokeSettings;
}

void VectorPath::startFillTransform()
{
    mSavedFillPaintSettings = mFillPaintSettings;
}

void VectorPath::finishStrokeTransform()
{
    addUndoRedo(new StrokeSettingsChangedUndoRedo(mSavedStrokeSettings,
                                                  mStrokeSettings,
                                                  this) );
}

void VectorPath::finishFillTransform()
{
    addUndoRedo(new FillSettingsChangedUndoRedo(mSavedFillPaintSettings,
                                                mFillPaintSettings,
                                                this) );
}

QRectF VectorPath::getBoundingRect()
{
    return mMappedWhole.boundingRect();
}

void VectorPath::draw(QPainter *p)
{
    if(mVisible) {
        p->save();
        p->setPen(Qt::NoPen);
        if(mFillPaintSettings.paintType == GRADIENTPAINT) {
            p->setBrush(mDrawFillGradient);
        } else if(mFillPaintSettings.paintType == FLATPAINT) {
            p->setBrush(mFillPaintSettings.color.qcol);
        } else{
            p->setBrush(Qt::NoBrush);
        }
        p->drawPath(mMappedPath);
        if(mStrokeSettings.paintType == GRADIENTPAINT) {
            p->setBrush(mDrawStrokeGradient);
        } else if(mStrokeSettings.paintType == FLATPAINT) {
            p->setBrush(mStrokeSettings.color.qcol);
        } else{
            p->setBrush(Qt::NoBrush);
        }
        p->drawPath(mOutlinePath);
        p->restore();
    }
}

void VectorPath::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    if(mVisible) {
        p->save();
        drawBoundingRect(p);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            p->setPen(QPen(QColor(0, 0, 0, 125), 2));
            foreach (PathPoint *point, mPoints) {
                point->draw(p, currentCanvasMode);
            }
            mFillGradientPoints.draw(p);
            mStrokeGradientPoints.draw(p);
        } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
            p->setPen(QPen(QColor(0, 0, 0, 125), 2));
            foreach (PathPoint *point, mPoints) {
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
        foreach (PathPoint *point, mPoints) {
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
}

PathPoint *VectorPath::addPoint(PathPoint *pointToAdd, PathPoint *toPoint)
{
    startNewUndoRedoSet();
    appendToPointsList(pointToAdd);
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
    point->show();
    if(saveUndoRedo) {
        AppendToPointsListUndoRedo *undoRedo = new AppendToPointsListUndoRedo(point, this);
        addUndoRedo(undoRedo);
    }
}

void VectorPath::removeFromPointsList(PathPoint *point, bool saveUndoRedo) {
    mPoints.removeOne(point);
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
    QPointF fillStartPt = mFillGradientPoints.startPoint->getRelativePos();
    QPointF fillEndPt = mFillGradientPoints.endPoint->getRelativePos();
    QPointF strokeStartPt = mStrokeGradientPoints.startPoint->getRelativePos();
    QPointF strokeEndPt = mStrokeGradientPoints.endPoint->getRelativePos();


    int fillSettingsId = mFillPaintSettings.saveToSql();
    int strokeSettingsId = mStrokeSettings.saveToSql();
    if(!query.exec(QString("INSERT INTO vectorpath (fillgradientstartx, fillgradientstarty, fillgradientendx, fillgradientendy, "
                        "strokegradientstartx, strokegradientstarty, strokegradientendx, strokegradientendy, "
                        "boundingboxid, fillsettingsid, strokesettingsid) "
                        "VALUES (%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11)").
                        arg(fillStartPt.x(), 0, 'f').
                        arg(fillStartPt.y(), 0, 'f').
                        arg(fillEndPt.x(), 0, 'f').
                        arg(fillEndPt.y(), 0, 'f').
                        arg(strokeStartPt.x(), 0, 'f').
                        arg(strokeStartPt.y(), 0, 'f').
                        arg(strokeEndPt.x(), 0, 'f').
                        arg(strokeEndPt.y(), 0, 'f').
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


GradientPoints::GradientPoints()
{

}

void GradientPoints::initialize(VectorPath *parentT,
                                QPointF startPt, QPointF endPt)
{
    parent = parentT;
    startPoint = new GradientPoint(startPt, parent);
    endPoint = new GradientPoint(endPt, parent);
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

void GradientPoints::draw(QPainter *p)
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
