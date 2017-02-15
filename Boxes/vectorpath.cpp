#include "Boxes/vectorpath.h"
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
int VectorPath::saveToSql(QSqlQuery *query, int parentId)
{
    int boundingBoxId = PathBox::saveToSql(query, parentId);
    foreach(PathPoint *point, mSeparatePaths) {
        point->saveToSql(query, boundingBoxId);
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

void VectorPath::applyCurrentTransformation() {
    applyTransformToPoints(mTransformAnimator.getCurrentValue());

    mTransformAnimator.reset(true);
    centerPivotPosition(true);
}

void VectorPath::applyTransformToPoints(QMatrix transform) {
    foreach(PathPoint *point, mPoints) {
        point->applyTransform(transform);
    }
}

void VectorPath::disconnectPoints(PathPoint *point1,
                                  PathPoint *point2) {
    if(point1->getNextPoint() != point2 &&
       point1->getPreviousPoint() != point2) return;
    if(point1->getPreviousPoint() == point2) {
        PathPoint *point2T = point2;
        point2 = point1;
        point1 = point2T;
    }
    point1->disconnectFromPoint(point2);

    if(mSeparatePaths.contains(point2)) return;
    bool stillConnected = false;
    PathPoint *currPoint = point1;
    PathPoint *prevPoint = point1->getPreviousPoint();
    PathPoint *firstPoint;
    while(prevPoint != NULL) {
        if(currPoint->isSeparatePathPoint()) {
            firstPoint = currPoint;
        }
        currPoint = prevPoint;
        prevPoint = currPoint->getPreviousPoint();
    }
    stillConnected = currPoint == point2;
    if(stillConnected) {
        removePointFromSeparatePaths(firstPoint);
    }
    addPointToSeparatePaths(point2);

    updatePathPointIds();
    schedulePathUpdate();
}

void VectorPath::connectPoints(PathPoint *point1,
                               PathPoint *point2) {
    PathPoint *point1FirstPoint = point1;
    PathPoint *point2FirstPoint = point2;
    while(point1FirstPoint->getNextPoint() != NULL) {
        point1FirstPoint = point1FirstPoint->getNextPoint();
    }
    while(point2FirstPoint->getNextPoint() != NULL) {
        point2FirstPoint = point2FirstPoint->getNextPoint();
    }
    if(point1FirstPoint == point2FirstPoint) {
        point1->connectToPoint(point2);
        return;
    }

    if(point1->isSeparatePathPoint() &&
       point2->isSeparatePathPoint()) {
        point1->reversePointsDirection();

        removePointFromSeparatePaths(point1);
        removePointFromSeparatePaths(point2);

        point1->connectToPoint(point2);
        PathPoint *firstPtCandidate = point1->getConnectedSeparatePathPoint();
        addPointToSeparatePaths(firstPtCandidate);
        qDebug() << "1";
    } else if(point1->isSeparatePathPoint()) {
        removePointFromSeparatePaths(point1);
        point1->connectToPoint(point2);
        qDebug() << "2";
    } else if(point2->isSeparatePathPoint()) {
        removePointFromSeparatePaths(point2);
        point1->connectToPoint(point2);
        qDebug() << "3";
    } else {
        PathPoint *point1ConnectedFirst =
                point1->getConnectedSeparatePathPoint();
        removePointFromSeparatePaths(point1ConnectedFirst);
        point1ConnectedFirst->reversePointsDirection();
        point1->connectToPoint(point2);
        qDebug() << "4";
    }

    updatePathPointIds();
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

    if(mSelected) {
        getMainWindow()->updateDisplayedShapesInMenu();
    }
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

    if(mSelected) {
        getMainWindow()->updateDisplayedShapesInMenu();
    }
}

void VectorPath::editShape(VectorPathShape *shape) {
    foreach(PathPoint *point, mPoints) {
        point->editShape(shape);
    }
    createDetachedUndoRedoStack();
}

void VectorPath::finishEditingShape(VectorPathShape *shape) {
    deleteDetachedUndoRedoStack();
    foreach(PathPoint *point, mPoints) {
        point->finishEditingShape(shape);
    }
}

void VectorPath::cancelEditingShape() {
    deleteDetachedUndoRedoStack();
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
                                      PathPoint **prevPoint,
                                      qreal *error) {
    PathPoint *currentPoint = NULL;
    PathPoint *nextPoint = NULL;
    qreal bestTVal = 0.;
    qreal minError = 10000000.;
    foreach(PathPoint *separatePoint, mSeparatePaths) {
        currentPoint = separatePoint;
        while(currentPoint->hasNextPoint() &&
              nextPoint != separatePoint) {
            nextPoint = currentPoint->getNextPoint();
            QRectF rect = qRectF4Points(currentPoint->getRelativePos(),
                                        currentPoint->getEndCtrlPtValue(),
                                        nextPoint->getStartCtrlPtValue(),
                                        nextPoint->getRelativePos());
            if(rect.adjusted(-15., -15, 15., 15.).contains(point)) {
                qreal error;
                qreal tVal = getTforBezierPoint(currentPoint->getRelativePos(),
                                                currentPoint->getEndCtrlPtValue(),
                                                nextPoint->getStartCtrlPtValue(),
                                                nextPoint->getRelativePos(),
                                                point,
                                                &error);
                if(error < minError && tVal > 0. && tVal < 1.) {
                    bestTVal = tVal;
                    minError = error;
                    *prevPoint = currentPoint;
                }
            }
            currentPoint = nextPoint;
        }
    }
    *error = minError;

    return bestTVal;
//    qreal smallestStep = 0.00001;
//    QPointF nearestPoint;
//    qreal smallestDist = 1000000.;
//    qreal nearestPercent = minPercent;
//    qreal percentStep = (maxPercent - minPercent)*0.01;
//    if(percentStep < smallestStep) return (maxPercent + minPercent)*0.5;
//    qreal currPercent = minPercent;
//    while(currPercent < maxPercent) {
//        QPointF testPoint = mEditPath.pointAtPercent(currPercent);
//        qreal dist = distBetweenTwoPoints(testPoint, point);
//        if(dist < smallestDist) {
//            smallestDist = dist;
//            nearestPoint = testPoint;
//            nearestPercent = currPercent;
//        }
//        currPercent += percentStep;
//    }
//    return findPercentForPoint(point,
//                               qclamp(nearestPercent - percentStep, 0., 1.),
//                               qclamp(nearestPercent + percentStep, 0., 1.) );
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

PathPoint *VectorPath::createNewPointOnLineNear(QPointF absPos,
                                                bool adjust) {
    qreal pressedT;
    PathPoint *prevPoint = NULL;
    PathPoint *nextPoint = NULL;
    if(getTAndPointsForMouseEdgeInteraction(absPos, &pressedT,
                                            &prevPoint, &nextPoint)) {
        if(pressedT > 0.0001 && pressedT < 0.9999) {
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

            return newPoint;
        }
    }
    return NULL;
}

bool doesPathIntersectWithCircle(const QPainterPath &path,
                                 qreal xRadius, qreal yRadius,
                                 QPointF center) {
    QPainterPath circlePath;
    circlePath.addEllipse(center, xRadius, yRadius);
    return circlePath.intersects(path);
}

bool doesPathNotContainCircle(const QPainterPath &path,
                              qreal xRadius, qreal yRadius,
                              QPointF center) {
    QPainterPath circlePath;
    circlePath.addEllipse(center, xRadius, yRadius);
    return !path.contains(circlePath);
}

QPointF getCenterOfPathIntersectionWithCircle(const QPainterPath &path,
                                              qreal xRadius, qreal yRadius,
                                              QPointF center) {
    QPainterPath circlePath;
    circlePath.addEllipse(center, xRadius, yRadius);
    return circlePath.intersected(path).boundingRect().center();
}

QPointF getCenterOfPathDifferenceWithCircle(const QPainterPath &path,
                                            qreal xRadius, qreal yRadius,
                                            QPointF center) {
    QPainterPath circlePath;
    circlePath.addEllipse(center, xRadius, yRadius);
    return circlePath.subtracted(path).boundingRect().center();
}

QPointF getPointClosestOnPathTo(const QPainterPath &path,
                                QPointF relPos,
                                qreal xRadiusScaling,
                                qreal yRadiusScaling) {
    bool (*checkerFunc)(const QPainterPath &, qreal, qreal, QPointF);
    QPointF (*centerFunc)(const QPainterPath &, qreal, qreal, QPointF);
    if(path.contains(relPos)) {
        checkerFunc = &doesPathNotContainCircle;
        centerFunc = &getCenterOfPathDifferenceWithCircle;
    } else {
        checkerFunc = &doesPathIntersectWithCircle;
        centerFunc = &getCenterOfPathIntersectionWithCircle;
    }
    qreal radius = 1.;
    while(true) {
        if(checkerFunc(path,
                       xRadiusScaling*radius,
                       yRadiusScaling*radius,
                       relPos)) {
            return centerFunc(path,
                              xRadiusScaling*radius,
                              yRadiusScaling*radius,
                              relPos);
        }
        radius += 1.;
    }
}

bool VectorPath::getTAndPointsForMouseEdgeInteraction(const QPointF &absPos,
                                                      qreal *pressedT,
                                                      PathPoint **prevPoint,
                                                      PathPoint **nextPoint) {
    qreal xScaling = mCombinedTransformMatrix.map(
                        QLineF(0., 0., 1., 0.)).length();
    qreal yScaling = mCombinedTransformMatrix.map(
                        QLineF(0., 0., 0., 1.)).length();
    qreal maxDistX = 8./xScaling;
    qreal maxDistY = 8./yScaling;
    QPointF relPos = mapAbsPosToRel(absPos);
    QRectF distRect = QRectF(relPos - QPointF(maxDistX, maxDistY),
                             QSizeF(maxDistX*2, maxDistY*2));
    if(!mEditPath.intersects(distRect) ||
        mEditPath.contains(distRect)) {
        return false;
    }

    relPos = getPointClosestOnPathTo(mEditPath, relPos,
                                     1./xScaling,
                                     1./yScaling);


    qreal error;
    *pressedT = findPercentForPoint(relPos, prevPoint, &error);
    if(*prevPoint == NULL) return false;

    *nextPoint = (*prevPoint)->getNextPoint();
    if(*nextPoint == NULL) return false;

    return true;
}

Edge *VectorPath::getEgde(QPointF absPos)
{
    qreal pressedT;
    PathPoint *prevPoint = NULL;
    PathPoint *nextPoint = NULL;
    if(getTAndPointsForMouseEdgeInteraction(absPos, &pressedT,
                                         &prevPoint, &nextPoint)) {
        if(pressedT > 0.0001 && pressedT < 0.9999) {
            return new Edge(prevPoint, nextPoint, pressedT);
        } else {
            return NULL;
        }
    }
    return NULL;
}

QPointF VectorPath::getRelCenterPosition() {
    QPointF posSum = QPointF(0., 0.);
    int count = mPoints.length();
    if(count == 0) return posSum;
    foreach(PathPoint *point, mPoints) {
        posSum += point->getRelativePos();
    }
    return posSum/count;
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
    //mPath.setFillRule(Qt::WindingFill);

    mEditPath = QPainterPath();

    foreach (PathPoint *firstPointInPath, mSeparatePaths) {
        PathPoint *point = firstPointInPath;
        PathPointValues lastPointValues;
        lastPointValues = point->getShapesInfluencedPointValues();
        mPath.moveTo(lastPointValues.pointRelPos);
        while(true) {
            point = point->getNextPoint();
            if(point == NULL) break;
            PathPointValues pointValues;

            pointValues = point->getShapesInfluencedPointValues();

            mPath.cubicTo(lastPointValues.endRelPos,
                          pointValues.startRelPos,
                          pointValues.pointRelPos);

            lastPointValues = pointValues;

            if(point == firstPointInPath) break;
        }

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

    updateOutlinePath();
}

void VectorPath::loadPathFromQPainterPath(const QPainterPath &path) {
    PathPoint *firstPoint = NULL;
    PathPoint *lastPoint = NULL;
    bool firstOther = true;
    QPointF startCtrlPoint;

    for(int i = 0; i < path.elementCount(); i++) {
        const QPainterPath::Element &elem = path.elementAt(i);

        if (elem.isMoveTo()) { // move
            lastPoint = addPointRelPos(QPointF(elem.x, elem.y), NULL);
            firstPoint = lastPoint;
        } else if (elem.isLineTo()) { // line
            if((QPointF(elem.x, elem.y) == firstPoint->getRelativePos()) ?
                    ((path.elementCount() > i + 1) ?
                                path.elementAt(i + 1).isMoveTo() :
                                true) :
                    false) {
                lastPoint->connectToPoint(firstPoint);
                lastPoint = firstPoint;
            } else {
                lastPoint = addPointRelPos(QPointF(elem.x, elem.y),
                                           lastPoint);
            }
        } else if (elem.isCurveTo()) { // curve
            lastPoint->setEndCtrlPtEnabled(true);
            lastPoint->moveEndCtrlPtToRelPos(QPointF(elem.x, elem.y));
            firstOther = true;
        } else { // other
            if(firstOther) {
                startCtrlPoint = QPointF(elem.x, elem.y);
            } else {
                if((QPointF(elem.x, elem.y) == firstPoint->getRelativePos()) ?
                        ((path.elementCount() > i + 1) ?
                                    path.elementAt(i + 1).isMoveTo() :
                                    true) :
                        false) {
                    lastPoint->connectToPoint(firstPoint);
                    lastPoint = firstPoint;
                } else {
                    lastPoint = addPointRelPos(QPointF(elem.x, elem.y),
                                               lastPoint);
                }
                lastPoint->setStartCtrlPtEnabled(true);
                lastPoint->moveStartCtrlPtToRelPos(startCtrlPoint);
            }
            firstOther = !firstOther;
        }
    }
}

PathPoint *VectorPath::addPointRelPos(QPointF relPos,
                                      QPointF startRelPos,
                                      QPointF endRelPos,
                                      PathPoint *toPoint) {
    PathPoint *newPoint = addPointRelPos(relPos, toPoint);
    //newPoint->setCtrlsMode(CTRLS_SYMMETRIC);
    newPoint->setStartCtrlPtEnabled(true);
    newPoint->setEndCtrlPtEnabled(true);
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

#include <QMenu>
void VectorPath::showContextMenu(QPoint globalPos) {
    QMenu menu(mMainWindow);

    QAction *outlineScaled = new QAction("Scale outline");
    outlineScaled->setCheckable(true);
    outlineScaled->setChecked(mOutlineAffectedByScale);
    menu.addAction(outlineScaled);

    menu.addAction("Delete");
    QAction *selected_action = menu.exec(globalPos);
    if(selected_action != NULL)
    {
        if(selected_action->text() == "Delete")
        {

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

    pointToRemove->removeFromVectorPath();

    Edge newEdge = Edge(prevPoint, nextPoint, 0.5);
    newEdge.makePassThrough(absPos);
}

void VectorPath::drawSelected(QPainter *p,
                              const CanvasMode &currentCanvasMode) {
    if(mVisible) {
        p->save();
        drawBoundingRect(p);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            p->save();
            p->setBrush(Qt::NoBrush);
            QPen editPen = QPen(Qt::white, 1., Qt::DashLine);
            editPen.setCosmetic(true);
            p->setPen(editPen);
            p->setTransform(QTransform(mCombinedTransformMatrix), true);
            p->setCompositionMode(QPainter::CompositionMode_Difference);
            p->drawPath(mEditPath);
            p->restore();

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

MovablePoint *VectorPath::getPointAt(const QPointF &absPtPos, const CanvasMode &currentCanvasMode)
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
    getParentCanvas()->removePointFromSelection(point);
    if(saveUndoRedo) {
        RemoveFromPointsListUndoRedo *undoRedo =
                new RemoveFromPointsListUndoRedo(point, this);
        addUndoRedo(undoRedo);
        if(mPoints.count() < 2) {
            mParent->removeChild(this);
        }
    }
    point->decNumberPointers();

    schedulePathUpdate();

    updatePathPointIds();
}

void VectorPath::removePoint(PathPoint *point) {
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
}

void VectorPath::replaceSeparatePathPoint(PathPoint *pointBeingReplaced,
                                          PathPoint *newPoint) {
    removePointFromSeparatePaths(pointBeingReplaced);
    addPointToSeparatePaths(newPoint);
}

void VectorPath::startAllPointsTransform()
{
    foreach(PathPoint *point, mPoints) {
        point->startTransform();
    }
}

void VectorPath::finishAllPointsTransform() {
    foreach(PathPoint *point, mPoints) {
        point->finishTransform();
    }
}

void VectorPath::duplicatePathPointsTo(
        VectorPath *target) {
    foreach(PathPoint *sepPoint, mSeparatePaths) {
        PathPoint *currPoint = sepPoint;
        PathPoint *lastAddedPoint = NULL;
        while(true) {
            PathPoint *pointToAdd = new PathPoint(target);
            currPoint->makeDuplicate(pointToAdd);
            target->addPoint(pointToAdd, lastAddedPoint);
            lastAddedPoint = pointToAdd;
            PathPoint *nextPoint = currPoint->getNextPoint();
            if(nextPoint == sepPoint) {
                currPoint->connectToPoint(sepPoint);
                break;
            }
            if(nextPoint == NULL) break;
            currPoint = nextPoint;
        }
    }
}
