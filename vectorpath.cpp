#include "vectorpath.h"
#include <QPainter>
#include "canvas.h"
#include <QDebug>
#include "undoredo.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "pathpivot.h"
#include "pointhelpers.h"

VectorPath::VectorPath(BoxesGroup *group) :
    PathBox(group, BoundingBoxType::TYPE_VECTOR_PATH)
{
    setName("Path");
    addActiveAnimator(&mPathAnimator);
    mAnimatorsCollection.addAnimator(&mPathAnimator);

    mPathAnimator.blockPointer();
}

#include <QSqlError>
int VectorPath::saveToSql(int parentId)
{
    int boundingBoxId = PathBox::saveToSql(parentId);
    foreach(PathPoint *point, mSeparatePaths) {
        point->saveToSql(boundingBoxId);
    }

    return boundingBoxId;
}


void VectorPath::loadFromSql(int boundingBoxId) {
    PathBox::loadFromSql(boundingBoxId);
    loadPointsFromSql(boundingBoxId);
    if(!mPivotChanged) centerPivotPosition();
}

VectorPath *VectorPath::createPathFromSql(int boundingBoxId,
                                          BoxesGroup *parent) {
    VectorPath *path = new VectorPath(parent);
    path->loadFromSql(boundingBoxId);

    return path;
}

VectorPath::~VectorPath()
{
    foreach(PathPoint *point, mPoints) {
        point->decNumberPointers();
    }
}

void VectorPath::applyCurrentTransformationToPoints() {
    startNewUndoRedoSet();

    mTransformAnimator.startTransform();
    applyTransformToPoints(mTransformAnimator.getCurrentValue());
    mTransformAnimator.finishTransform();

    mTransformAnimator.reset(true);
    centerPivotPosition(true);

    finishUndoRedoSet();
}

void VectorPath::applyTransformToPoints(QMatrix transform) {
    foreach(PathPoint *point, mPoints) {
        point->applyTransform(transform);
    }
}

void VectorPath::loadPointsFromSql(int boundingBoxId) {
    QSqlQuery query;
    QString queryStr = QString("SELECT id, isfirst, isendpoint, movablepointid "
                               "FROM pathpoint WHERE boundingboxid = %1 "
                               "ORDER BY id ASC").arg(boundingBoxId);
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

            PathPoint *newPoint = new PathPoint(this);
            newPoint->loadFromSql(id, movablepointid);
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
        qDebug() << "Could not load points for vectorpath with id " << boundingBoxId;
    }
}

#include "vectorshapesmenu.h"
void VectorPath::addShapesToShapesMenu(VectorShapesMenu *menu) {
    foreach(VectorPathShape *shape, mShapes) {
        menu->addShapeWidgetForShape(shape);
    }
}

VectorPathShape *VectorPath::createNewShape(bool relative) {
    VectorPathShape *shape = new VectorPathShape();
    shape->setRelative(relative);
    shape->setName("shape " + QString::number(mShapes.count() + 1));

    shape->setNumberPoints(mPoints.count());
    addShape(shape);

    foreach(PathPoint *point, mPoints) {
        point->saveInitialPointValuesToShapeValues(shape);
    }

    return shape;
}

void VectorPath::addShape(VectorPathShape *shape, bool saveUndoRedo) {
    mShapesEnabled = true;
    if(mShapesAnimator == NULL) {
        mShapesAnimator = new ComplexAnimator();
        mShapesAnimator->setName("shapes");
        mShapesAnimator->blockPointer();
        addActiveAnimator(mShapesAnimator);
    }

    if(saveUndoRedo) {
        addUndoRedo(new AddShapeUndoRedo(this, shape));
    }

    mShapesAnimator->addChildAnimator(shape->getInfluenceAnimator());
    shape->incNumberPointers();
    PathPointUpdater *updater = new PathPointUpdater(this);
    shape->getInfluenceAnimator()->setUpdater(updater);

    mShapes << shape;
    foreach(PathPoint *point, mPoints) {
        point->addShapeValues(shape);
    }

    schedulePathUpdate();
}

void VectorPath::removeShape(VectorPathShape *shape, bool saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new RemoveShapeUndoRedo(this, shape));
    }
    shape->getInfluenceAnimator()->setUpdater(NULL);
    mShapes.removeOne(shape);
    foreach(PathPoint *point, mPoints) {
        point->removeShapeValues(shape);
    }
    mShapesAnimator->removeChildAnimator(shape->getInfluenceAnimator());
    if(mShapes.isEmpty()) {
        removeActiveAnimator(mShapesAnimator);
        delete mShapesAnimator;
        mShapesAnimator = NULL;
        mShapesEnabled = false;
    }
    shape->decNumberPointers();


    schedulePathUpdate();
}

void VectorPath::editShape(VectorPathShape *shape) {
    foreach(PathPoint *point, mPoints) {
        point->editShape(shape);
    }
}

void VectorPath::finishEditingShape(VectorPathShape *shape) {
    foreach(PathPoint *point, mPoints) {
        point->finishEditingShape(shape);
    }
}

void VectorPath::cancelEditingShape() {
    foreach(PathPoint *point, mPoints) {
        point->cancelEditingShape();
    }
}

void VectorPath::saveCurrentPathToShape(VectorPathShape *shape) {
    foreach(PathPoint *point, mPoints) {
        point->savePointValuesToShapeValues(shape);
    }
}

qreal distBetweenTwoPoints(QPointF point1, QPointF point2) {
    QPointF dPoint = point1 - point2;
    return sqrt(dPoint.x()*dPoint.x() + dPoint.y()*dPoint.y());
}

qreal VectorPath::findPercentForPoint(QPointF point,
                                      qreal minPercent, qreal maxPercent) {
    qreal smallestStep = 0.00001;
    QPointF nearestPoint;
    qreal smallestDist = 1000000.;
    qreal nearestPercent = minPercent;
    qreal percentStep = (maxPercent - minPercent)*0.01;
    if(percentStep < smallestStep) return (maxPercent + minPercent)*0.5;
    qreal currPercent = minPercent;
    while(currPercent < maxPercent) {
        QPointF testPoint = mPath.pointAtPercent(currPercent);
        qreal dist = distBetweenTwoPoints(testPoint, point);
        if(dist < smallestDist) {
            smallestDist = dist;
            nearestPoint = testPoint;
            nearestPercent = currPercent;
        }
        currPercent += percentStep;
    }
    return findPercentForPoint(point,
                               qclamp(nearestPercent - percentStep, 0., 1.),
                               qclamp(nearestPercent + percentStep, 0., 1.) );
}

qreal VectorPath::findPercentForPointEditPath(QPointF point,
                                      qreal minPercent, qreal maxPercent) {
    qreal smallestStep = 0.00001;
    QPointF nearestPoint;
    qreal smallestDist = 1000000.;
    qreal nearestPercent = minPercent;
    qreal percentStep = (maxPercent - minPercent)*0.01;
    if(percentStep < smallestStep) return (maxPercent + minPercent)*0.5;
    qreal currPercent = minPercent;
    while(currPercent < maxPercent) {
        QPointF testPoint = mEditPath.pointAtPercent(currPercent);
        qreal dist = distBetweenTwoPoints(testPoint, point);
        if(dist < smallestDist) {
            smallestDist = dist;
            nearestPoint = testPoint;
            nearestPercent = currPercent;
        }
        currPercent += percentStep;
    }
    return findPercentForPointEditPath(point,
                                       qclamp(nearestPercent - percentStep, 0., 1.),
                                       qclamp(nearestPercent + percentStep, 0., 1.) );
}

PathPoint *VectorPath::findPointNearestToPercent(qreal percent,
                                                 qreal *foundAtPercent) {
    PathPoint *nearestPoint = mPoints.first();
    qreal nearestPointPercent = 100.;
    foreach(PathPoint *point, mPoints) {
        qreal pointPercent = findPercentForPoint(point->getRelativePos());
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
        qreal pointPercent = findPercentForPoint(point->getInfluenceRelativePos());
        if(qAbs(pointPercent - percent) < qAbs(nearestPointPercent - percent)) {
            nearestPointPercent = pointPercent;
            nearestPoint = point;
        }
    }
    *foundAtPercent = nearestPointPercent;
    return nearestPoint;
}

void VectorPath::updateAfterFrameChanged(int currentFrame)
{
    foreach(PathPoint *point, mPoints) {
        point->updateAfterFrameChanged(currentFrame);
    }
    if(mShapesAnimator != NULL) {
        mShapesAnimator->setFrame(currentFrame);
    }
    mPathAnimator.setFrame(currentFrame);
    PathBox::updateAfterFrameChanged(currentFrame);
}

PathPoint *VectorPath::createNewPointOnLineNear(QPointF absPos, bool adjust)
{
    qreal maxDist = 14.;
    QPointF relPos = mapAbsPosToRel(absPos);
    if(!mPath.intersects(QRectF(relPos - QPointF(maxDist, maxDist),
                                     QSizeF(maxDist*2, maxDist*2))) ) {
        return NULL;
    }

    qreal nearestPercent = findPercentForPoint(relPos);
    QPointF nearestPtOnPath = mPath.pointAtPercent(nearestPercent);
    if((nearestPtOnPath - relPos).manhattanLength() > maxDist ) {
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

    if(nextPoint == NULL || prevPoint == NULL) return NULL;

    qreal percent1 = findPercentForPoint(prevPoint->getRelativePos());
    qreal percent2 = findPercentForPoint(nextPoint->getRelativePos());
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

        PathPoint *newPoint = new PathPoint(this);
        newPoint->setRelativePos(newPointPos, false);

        nextPoint->setPointAsPrevious(newPoint);
        prevPoint->setPointAsNext(newPoint);

        if(adjust) {
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
        }

        appendToPointsList(newPoint);

        finishUndoRedoSet();
        return newPoint;
    }
    return NULL;
}

Edge *VectorPath::getEdgeFromPath(QPointF absPos) {
    qreal maxDist = 8.;
    QPointF relPos = mapAbsPosToRel(absPos);
    if(!mPath.intersects(QRectF(relPos - QPointF(maxDist, maxDist),
                                     QSizeF(maxDist*2, maxDist*2))) ) {
        return NULL;
    }

    qreal nearestPercent = findPercentForPoint(relPos);
    QPointF nearestPtOnPath = mPath.pointAtPercent(nearestPercent);

    if((nearestPtOnPath - relPos).manhattanLength() > maxDist ) {
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

    if(mInfluenceEnabled) {
        while(!nextPoint->hasFullInfluence() ) {
            nextPoint = nextPoint->getNextPoint();
            if(nextPoint == nextPointS || nextPoint == NULL) return NULL;
        }

        while(!prevPoint->hasFullInfluence() ) {
            prevPoint = prevPoint->getPreviousPoint();
            if(prevPoint == prevPointS || prevPoint == NULL) return NULL;
        }
    }

    qreal percent1 = findPercentForPoint(prevPoint->getRelativePos());
    qreal percent2 = findPercentForPoint(nextPoint->getRelativePos());
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

Edge *VectorPath::getEdgeFromEditPath(QPointF absPos)
{
    qreal maxDist = 8.;
    QPointF relPos = mapAbsPosToRel(absPos);
    if(!mEditPath.intersects(QRectF(relPos - QPointF(maxDist, maxDist),
                                     QSizeF(maxDist*2, maxDist*2))) ) {
        return NULL;
    }

    qreal nearestPercent = findPercentForPointEditPath(relPos);
    QPointF nearestPtOnPath = mEditPath.pointAtPercent(nearestPercent);

    if((nearestPtOnPath - relPos).manhattanLength() > maxDist ) {
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

    qreal percent1 = findPercentForPointEditPath(prevPoint->getRelativePos());
    qreal percent2 = findPercentForPointEditPath(nextPoint->getRelativePos());
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
    if(mInfluenceEnabled || mShapesEnabled) {
        edgeT = getEdgeFromEditPath(absPos);
    }
    if(edgeT == NULL) {
        edgeT = getEdgeFromPath(absPos);
    }
    return edgeT;
}

void VectorPath::centerPivotPosition(bool finish) {
    QPointF posSum = QPointF(0., 0.);
    int count = mPoints.length();
    if(count == 0) return;
    foreach(PathPoint *point, mPoints) {
        posSum += point->getRelativePos();
    }
    mTransformAnimator.setPivotWithoutChangingTransformation(posSum/count, finish);
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

    //if(mInfluenceEnabled || mShapesEnabled || !mEffects.isEmpty()) {
        mEditPath = QPainterPath();
    if(mInfluenceEnabled) {
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
    //}

    foreach (PathPoint *firstPointInPath, mSeparatePaths) {
        PathPoint *point = firstPointInPath;
        PathPointValues lastPointValues;
        if(mInfluenceEnabled) {
            lastPointValues = point->getInfluenceAdjustedPointValues();
        } else {
            lastPointValues = point->getShapesInfluencedPointValues();
        }
        mPath.moveTo(lastPointValues.pointRelPos);
        while(true) {
            point = point->getNextPoint();
            if(point == NULL) break;
            PathPointValues pointValues;

            if(mInfluenceEnabled) {
                pointValues = point->getInfluenceAdjustedPointValues();
            } else {
                pointValues = point->getShapesInfluencedPointValues();
            }

            mPath.cubicTo(lastPointValues.endRelPos,
                          pointValues.startRelPos,
                          pointValues.pointRelPos);

            lastPointValues = pointValues;

            if(point == firstPointInPath) break;
        }

        //if(mInfluenceEnabled || mShapesEnabled || !mEffects.isEmpty()) {
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
        //}
    }

    updateOutlinePath();
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
    newPoint->setCtrlsMode(CTRLS_SYMMETRIC);
    newPoint->moveStartCtrlPtToRelPos(startRelPos);
    newPoint->moveEndCtrlPtToRelPos(endRelPos);
    return newPoint;
}

void VectorPath::updatePathPointIds()
{
    int pointId = 0;
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

    QAction *outlineScaled = new QAction("Scale outline");
    outlineScaled->setCheckable(true);
    outlineScaled->setChecked(mOutlineAffectedByScale);
    menu.addAction(outlineScaled);

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
        } else if(selected_action == outlineScaled) {
            setOutlineAffectedByScale(!mOutlineAffectedByScale);
        }
    } else {

    }
}

void VectorPath::deletePointAndApproximate(PathPoint *pointToRemove) {
    PathPoint *nextPoint = pointToRemove->getNextPoint();
    PathPoint *prevPoint = pointToRemove->getPreviousPoint();
    if(nextPoint == NULL || prevPoint == NULL) return;

    QPointF absPos = pointToRemove->getAbsolutePos();

    pointToRemove->remove();

    Edge newEdge = Edge(prevPoint, nextPoint, 0.5);
    newEdge.makePassThrough(absPos);
}

void VectorPath::drawSelected(QPainter *p, CanvasMode currentCanvasMode) {
    if(mVisible) {
        p->save();
        //drawBoundingRect(p);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            //if(mInfluenceEnabled || mShapesEnabled || !mEffects.isEmpty()) {
                p->save();
                p->setBrush(Qt::NoBrush);
                p->setPen(QPen(Qt::blue, 1., Qt::DashLine) );
                p->setTransform(QTransform(mCombinedTransformMatrix), true);
                p->drawPath(mEditPath);
                p->restore();
            //}
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
    PathPoint *newPoint = new PathPoint(this);
    newPoint->setAbsolutePos(absPtPos, false);
    newPoint->moveStartCtrlPtToAbsPos(absPtPos);
    newPoint->moveEndCtrlPtToAbsPos(absPtPos);

    return addPoint(newPoint, toPoint);
}

PathPoint *VectorPath::addPointRelPos(QPointF relPtPos, PathPoint *toPoint)
{
    PathPoint *newPoint = new PathPoint(this);
    newPoint->setRelativePos(relPtPos, false);
    newPoint->moveStartCtrlPtToRelPos(relPtPos);
    newPoint->moveEndCtrlPtToRelPos(relPtPos);

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

    schedulePathUpdate();

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

    schedulePathUpdate();

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

void VectorPath::replaceSeparatePathPoint(PathPoint *pointBeingReplaced,
                                          PathPoint *newPoint) {
    startNewUndoRedoSet();
    removePointFromSeparatePaths(pointBeingReplaced);
    addPointToSeparatePaths(newPoint);
    finishUndoRedoSet();
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
