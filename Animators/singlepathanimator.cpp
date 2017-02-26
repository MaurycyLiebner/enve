#include "Animators/singlepathanimator.h"
#include "pathanimator.h"
#include "pathpoint.h"
#include "undoredo.h"
#include "edge.h"

SinglePathAnimator::SinglePathAnimator(PathAnimator *parentPath) :
    ComplexAnimator() {
    setName("path");
    mParentPathAnimator = parentPath;
}

SinglePathAnimator::~SinglePathAnimator() {
    foreach(PathPoint *point, mPoints) {
        point->decNumberPointers();
    }
}

Edge *SinglePathAnimator::getEgde(QPointF absPos) {
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

QPointF SinglePathAnimator::getRelCenterPosition() {
    QPointF posSum = QPointF(0., 0.);
    int count = mPoints.length();
    if(count == 0) return posSum;
    foreach(PathPoint *point, mPoints) {
        posSum += point->getRelativePos();
    }
    return posSum/count;
}

void SinglePathAnimator::updatePathPointIds() {
    if(mFirstPoint == NULL) return;
    int pointId = 0;
    PathPoint *nextPoint = mFirstPoint;
    while(true) {
        nextPoint->setPointId(pointId);
        pointId++;
        nextPoint = nextPoint->getNextPoint();
        if(nextPoint == NULL || nextPoint == mFirstPoint) break;
    }
}

PathPoint *SinglePathAnimator::addPointRelPos(const QPointF &relPos,
                                        const QPointF &startRelPos,
                                        const QPointF &endRelPos,
                                        PathPoint *toPoint) {
    PathPoint *newPoint = addPointRelPos(relPos, toPoint);
    //newPoint->setCtrlsMode(CTRLS_SYMMETRIC);
    newPoint->setStartCtrlPtEnabled(true);
    newPoint->setEndCtrlPtEnabled(true);
    newPoint->moveStartCtrlPtToRelPos(startRelPos);
    newPoint->moveEndCtrlPtToRelPos(endRelPos);
    return newPoint;
}

//void SinglePathAnimator::loadPathFromQPainterPath(const QPainterPath &path) {
//    PathPoint *firstPoint = NULL;
//    PathPoint *lastPoint = NULL;
//    bool firstOther = true;
//    QPointF startCtrlPoint;

//    for(int i = 0; i < path.elementCount(); i++) {
//        const QPainterPath::Element &elem = path.elementAt(i);

//        if (elem.isMoveTo()) { // move
//            lastPoint = addPointRelPos(QPointF(elem.x, elem.y), NULL);
//            firstPoint = lastPoint;
//        } else if (elem.isLineTo()) { // line
//            if((QPointF(elem.x, elem.y) == firstPoint->getRelativePos()) ?
//                    ((path.elementCount() > i + 1) ?
//                                path.elementAt(i + 1).isMoveTo() :
//                                true) :
//                    false) {
//                lastPoint->connectToPoint(firstPoint);
//                lastPoint = firstPoint;
//            } else {
//                lastPoint = addPointRelPos(QPointF(elem.x, elem.y),
//                                           lastPoint);
//            }
//        } else if (elem.isCurveTo()) { // curve
//            lastPoint->setEndCtrlPtEnabled(true);
//            lastPoint->moveEndCtrlPtToRelPos(QPointF(elem.x, elem.y));
//            firstOther = true;
//        } else { // other
//            if(firstOther) {
//                startCtrlPoint = QPointF(elem.x, elem.y);
//            } else {
//                if((QPointF(elem.x, elem.y) == firstPoint->getRelativePos()) ?
//                        ((path.elementCount() > i + 1) ?
//                                    path.elementAt(i + 1).isMoveTo() :
//                                    true) :
//                        false) {
//                    lastPoint->connectToPoint(firstPoint);
//                    lastPoint = firstPoint;
//                } else {
//                    lastPoint = addPointRelPos(QPointF(elem.x, elem.y),
//                                               lastPoint);
//                }
//                lastPoint->setStartCtrlPtEnabled(true);
//                lastPoint->moveStartCtrlPtToRelPos(startCtrlPoint);
//            }
//            firstOther = !firstOther;
//        }
//    }
//}

void SinglePathAnimator::updatePath()
{
    mPath = QPainterPath();
    //mPath.setFillRule(Qt::WindingFill);

    PathPoint *point = mFirstPoint;
    PathPointValues lastPointValues;
    lastPointValues = point->getPointValues();
    mPath.moveTo(lastPointValues.pointRelPos);
    while(true) {
        point = point->getNextPoint();
        if(point == NULL) break;
        PathPointValues pointValues;

        pointValues = point->getPointValues();

        mPath.cubicTo(lastPointValues.endRelPos,
                      pointValues.startRelPos,
                      pointValues.pointRelPos);

        lastPointValues = pointValues;

        if(point == mFirstPoint) break;
    }
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

bool SinglePathAnimator::getTAndPointsForMouseEdgeInteraction(const QPointF &absPos,
                                                      qreal *pressedT,
                                                      PathPoint **prevPoint,
                                                      PathPoint **nextPoint) {
    const QMatrix &combinedTransform =
            mParentPathAnimator->getParentBox()->getCombinedTransform();
    qreal xScaling = combinedTransform.map(
                        QLineF(0., 0., 1., 0.)).length();
    qreal yScaling = combinedTransform.map(
                        QLineF(0., 0., 0., 1.)).length();
    qreal maxDistX = 4./xScaling;
    qreal maxDistY = 4./yScaling;
    QPointF relPos = combinedTransform.inverted().map(absPos);
    QRectF distRect = QRectF(relPos - QPointF(maxDistX, maxDistY),
                             QSizeF(maxDistX*2, maxDistY*2));
    if(!mPath.intersects(distRect) ||
        mPath.contains(distRect)) {
        return false;
    }

    relPos = getPointClosestOnPathTo(mPath, relPos,
                                     1./xScaling,
                                     1./yScaling);


    qreal error;
    *pressedT = findPercentForPoint(relPos, prevPoint, &error);
    if(*prevPoint == NULL) return false;

    *nextPoint = (*prevPoint)->getNextPoint();
    if(*nextPoint == NULL) return false;

    return true;
}


PathPoint *SinglePathAnimator::createNewPointOnLineNear(QPointF absPos,
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
                if(!prevPoint->isEndCtrlPtEnabled() &&
                   !nextPoint->isStartCtrlPtEnabled()) {
                    newPoint->setStartCtrlPtEnabled(false);
                    newPoint->setEndCtrlPtEnabled(false);
                } else {
                    newPoint->setCtrlsMode(CtrlsMode::CTRLS_SMOOTH, false);
                    newPoint->setStartCtrlPtEnabled(true);
                    newPoint->moveStartCtrlPtToRelPos(newPointStart);
                    newPoint->setEndCtrlPtEnabled(true);
                    newPoint->moveEndCtrlPtToRelPos(newPointEnd);

                    if(prevPoint->getCurrentCtrlsMode() ==
                       CtrlsMode::CTRLS_SYMMETRIC &&
                       prevPoint->isEndCtrlPtEnabled() &&
                       prevPoint->isStartCtrlPtEnabled()) {
                        prevPoint->setCtrlsMode(CtrlsMode::CTRLS_SMOOTH);
                    }
                    if(nextPoint->getCurrentCtrlsMode() ==
                       CtrlsMode::CTRLS_SYMMETRIC &&
                       nextPoint->isEndCtrlPtEnabled() &&
                       nextPoint->isStartCtrlPtEnabled()) {
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

void SinglePathAnimator::updateAfterFrameChanged(int currentFrame) {
    foreach(PathPoint *point, mPoints) {
        point->updateAfterFrameChanged(currentFrame);
    }
}

qreal distBetweenTwoPoints(QPointF point1, QPointF point2) {
    QPointF dPoint = point1 - point2;
    return sqrt(dPoint.x()*dPoint.x() + dPoint.y()*dPoint.y());
}

qreal SinglePathAnimator::findPercentForPoint(const QPointF &point,
                                      PathPoint **prevPoint,
                                      qreal *error) {
    PathPoint *currentPoint = NULL;
    PathPoint *nextPoint = NULL;
    qreal bestTVal = 0.;
    qreal minError = 10000000.;
    currentPoint = mFirstPoint;
    while(currentPoint->hasNextPoint() &&
          nextPoint != mFirstPoint) {
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
    *error = minError;

    return bestTVal;
}

void SinglePathAnimator::applyTransformToPoints(const QMatrix &transform) {
    foreach(PathPoint *point, mPoints) {
        point->applyTransform(transform);
    }
}

void SinglePathAnimator::disconnectPoints(PathPoint *point1,
                                          PathPoint *point2) {
    if(point1->getNextPoint() != point2 &&
       point1->getPreviousPoint() != point2) return;
    if(point1->getPreviousPoint() == point2) {
        PathPoint *point2T = point2;
        point2 = point1;
        point1 = point2T;
    }
    point1->disconnectFromPoint(point2);

    if(mFirstPoint == point2) return;
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
        replaceSeparatePathPoint(point2);
    } else {
        SinglePathAnimator *newPath = new SinglePathAnimator(
                                                mParentPathAnimator);
        PathPoint *currPt = point2;
        while(true) {
            removeFromPointsList(currPt);
            newPath->appendToPointsList(currPt);
            currPt = currPt->getNextPoint();
            if(currPt == point2 || currPt == NULL) break;
        }
        newPath->replaceSeparatePathPoint(point2);

        newPath->updatePathPointIds();
        mParentPathAnimator->addSinglePathAnimator(newPath);
    }

    updatePathPointIds();
    //schedulePathUpdate();
    callUpdater();
}

void SinglePathAnimator::connectPoints(PathPoint *point1,
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

    SinglePathAnimator *point1ParentPath = point1->getParentPath();
    SinglePathAnimator *point2ParentPath = point2->getParentPath();
    if(point1->isSeparatePathPoint() &&
       point2->isSeparatePathPoint()) {
        point1->reversePointsDirectionStartingFromThis();
        point1ParentPath->changeAllPointsParentPathTo(point2ParentPath);
        mParentPathAnimator->removeSinglePathAnimator(point1ParentPath);

        point2ParentPath->replaceSeparatePathPoint(NULL);

        point1->connectToPoint(point2);
        PathPoint *firstPtCandidate = point1->getConnectedSeparatePathPoint();
        point2ParentPath->replaceSeparatePathPoint(firstPtCandidate);
        point2ParentPath->updatePathPointIds();
    } else if(point1->isSeparatePathPoint()) {
        point1ParentPath->changeAllPointsParentPathTo(point2ParentPath);
        mParentPathAnimator->removeSinglePathAnimator(point1ParentPath);
        point1->connectToPoint(point2);
        point2ParentPath->updatePathPointIds();
    } else if(point2->isSeparatePathPoint()) {
        point2ParentPath->changeAllPointsParentPathTo(point1ParentPath);
        mParentPathAnimator->removeSinglePathAnimator(point2ParentPath);
        point1->connectToPoint(point2);
        point1ParentPath->updatePathPointIds();
    } else {
        PathPoint *point1ConnectedFirst =
                point1->getConnectedSeparatePathPoint();

        point1ConnectedFirst->reversePointsDirectionStartingFromThis();

        point1ParentPath->changeAllPointsParentPathTo(
                    point2ParentPath);
        mParentPathAnimator->removeSinglePathAnimator(
                    point1ParentPath);

        point1->connectToPoint(point2);
        point2ParentPath->updatePathPointIds();
    }

//    if(point1->isSeparatePathPoint() &&
//       point2->isSeparatePathPoint()) {
//        point1->reversePointsDirection();

//        removePointFromSeparatePaths(point1);
//        removePointFromSeparatePaths(point2);

//        point1->connectToPoint(point2);
//        PathPoint *firstPtCandidate = point1->getConnectedSeparatePathPoint();
//        addPointToSeparatePaths(firstPtCandidate);
//    } else if(point1->isSeparatePathPoint()) {
//        removePointFromSeparatePaths(point1);
//        point1->connectToPoint(point2);
//    } else if(point2->isSeparatePathPoint()) {
//        removePointFromSeparatePaths(point2);
//        point1->connectToPoint(point2);
//    } else {
//        PathPoint *point1ConnectedFirst =
//                point1->getConnectedSeparatePathPoint();
//        removePointFromSeparatePaths(point1ConnectedFirst);
//        point1ConnectedFirst->reversePointsDirection();
//        point1->connectToPoint(point2);
//    }

    //updatePathPointIds();
}

//void SinglePathAnimator::disconnectPoints(PathPoint *point1,
//                                          PathPoint *point2) {
//    if(point1->getNextPoint() != point2 &&
//       point1->getPreviousPoint() != point2) return;
//    if(point1->getPreviousPoint() == point2) {
//        PathPoint *point2T = point2;
//        point2 = point1;
//        point1 = point2T;
//    }
//    point1->disconnectFromPoint(point2);
//    PathPoint *point1SepPt = point1->getConnectedSeparatePathPoint();
//    PathPoint *point2SepPt = point2->getConnectedSeparatePathPoint();

//    if(point2 != mFirstPoint) {
//        if(point1 == mFirstPoint) {
//            replaceSeparatePathPoint(point2);
//        } else if(point1SepPt == point2SepPt) {
//            replaceSeparatePathPoint(point1SepPt);
//        } else {
//            SinglePathAnimator *newSinglePathAnimator =
//                                new SinglePathAnimator(mParentPathAnimator);
//            mParentPathAnimator->addSinglePathAnimator(newSinglePathAnimator);
//            newSinglePathAnimator->replaceSeparatePathPoint(point2);
//            PathPoint *currPoint = point2;
//            while(true) {
//                removeFromPointsList(currPoint);
//                newSinglePathAnimator->addPoint(currPoint);
//                currPoint = currPoint->getNextPoint();
//                if(currPoint == NULL || currPoint == point2) break;
//            }
//        }
//    }

//    updatePathPointIds();
//    //schedulePathUpdate();
//    callUpdater();
//}

//void SinglePathAnimator::connectPoints(PathPoint *point1,
//                                       PathPoint *point2) {
//    PathPoint *point1FirstPoint = point1->getConnectedSeparatePathPoint();
//    PathPoint *point2FirstPoint = point2->getConnectedSeparatePathPoint();

//    if(point1FirstPoint == point2FirstPoint) {
//        point1->connectToPoint(point2);
//    }

//    updatePathPointIds();
//}

void SinglePathAnimator::changeAllPointsParentPathTo(SinglePathAnimator *path) {
    replaceSeparatePathPoint(NULL);
    QList<PathPoint*> allPoints = mPoints;
    foreach(PathPoint *point, allPoints) {
        point->incNumberPointers();
        removeFromPointsList(point);
        path->appendToPointsList(point);
        point->decNumberPointers();
    }
    mPoints.clear();
}

void SinglePathAnimator::deletePointAndApproximate(PathPoint *pointToRemove) {
    PathPoint *nextPoint = pointToRemove->getNextPoint();
    PathPoint *prevPoint = pointToRemove->getPreviousPoint();
    if(nextPoint == NULL || prevPoint == NULL) return;

    QPointF absPos = pointToRemove->getAbsolutePos();

    pointToRemove->removeFromVectorPath();

    Edge newEdge = Edge(prevPoint, nextPoint, 0.5);
    newEdge.makePassThrough(absPos);
}

MovablePoint *SinglePathAnimator::getPointAt(const QPointF &absPtPos,
                                     const CanvasMode &currentCanvasMode)
{
    foreach(PathPoint *point, mPoints) {
        MovablePoint *pointToReturn =
                point->getPointAtAbsPos(absPtPos, currentCanvasMode);
        if(pointToReturn == NULL) continue;
        return pointToReturn;
    }
    return NULL;
}

void SinglePathAnimator::drawSelected(QPainter *p,
                                const CanvasMode &currentCanvasMode,
                                const QMatrix &combinedTransform) {
    p->save();
    if(currentCanvasMode == CanvasMode::MOVE_POINT) {
        p->save();
        p->setBrush(Qt::NoBrush);
        QPen editPen = QPen(Qt::white, 1., Qt::DashLine);
        editPen.setCosmetic(true);
        p->setPen(editPen);
        p->setTransform(QTransform(combinedTransform), true);
        p->setCompositionMode(QPainter::CompositionMode_Difference);
        p->drawPath(mPath);
        p->restore();

        p->setPen(QPen(QColor(0, 0, 0, 255), 1.5));
        PathPoint *point;
        foreachInverted(point, mPoints) {
            point->draw(p, currentCanvasMode);
        }
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

void SinglePathAnimator::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<MovablePoint *> *list) {
    foreach(PathPoint *point, mPoints) {
        point->rectPointsSelection(absRect, list);
    }
}

void SinglePathAnimator::savePointsToSql(QSqlQuery *query,
                                   const int &boundingBoxId) {
    mFirstPoint->saveToSql(query, boundingBoxId);
}

void SinglePathAnimator::loadPointsFromSql(int boundingBoxId) {
//    QSqlQuery query;
//    QString queryStr = QString("SELECT id, isfirst, isendpoint, movablepointid "
//                               "FROM pathpoint WHERE boundingboxid = %1 "
//                               "ORDER BY id ASC").arg(boundingBoxId);
//    if(query.exec(queryStr) ) {
//        int idisfirst = query.record().indexOf("isfirst");
//        int idisendpoint = query.record().indexOf("isendpoint");
//        int idmovablepointid = query.record().indexOf("movablepointid");
//        int idId = query.record().indexOf("id");
//        PathPoint *firstPoint = NULL;
//        PathPoint *lastPoint = NULL;
//        while(query.next()) {
//            int id = query.value(idId).toInt();
//            bool isfirst = query.value(idisfirst).toBool();
//            bool isendpoint = query.value(idisendpoint).toBool();
//            int movablepointid = query.value(idmovablepointid).toInt();

//            PathPoint *newPoint = new PathPoint(this);
//            newPoint->loadFromSql(id, movablepointid);
//            appendToPointsList(newPoint, false);
//            if(lastPoint != NULL) {
//                if(isfirst && firstPoint != NULL) {
//                    lastPoint->setPointAsNext(firstPoint, false);
//                } else if(!isfirst) {
//                    lastPoint->setPointAsNext(newPoint, false);
//                }
//            }
//            if(isfirst) {
//                addPointToSeparatePaths(newPoint, false);
//                if(isendpoint) {
//                    firstPoint = NULL;
//                } else {
//                    firstPoint = newPoint;
//                }
//            }
//            lastPoint = newPoint;
//        }
//        if(lastPoint != NULL && firstPoint != NULL) {
//            lastPoint->setPointAsNext(firstPoint, false);
//        }
//    } else {
//        qDebug() << "Could not load points for vectorpath with id " << boundingBoxId;
//    }
}

PathPoint *SinglePathAnimator::addPoint(PathPoint *pointToAdd,
                                        PathPoint *toPoint)
{
    if(toPoint == NULL) {
        replaceSeparatePathPoint(pointToAdd);
    } else {
        if(!toPoint->hasNextPoint()) {
            toPoint->setPointAsNext(pointToAdd);
        } else if(!toPoint->hasPreviousPoint() ) {
            replaceSeparatePathPoint(pointToAdd);
            toPoint->setPointAsPrevious(pointToAdd);
        }
    }

    appendToPointsList(pointToAdd);

    return pointToAdd;
}

PathPoint* SinglePathAnimator::addPointAbsPos(const QPointF &absPtPos,
                                        PathPoint *toPoint)
{
    PathPoint *newPoint = new PathPoint(this);
    newPoint->setAbsolutePos(absPtPos, false);
    newPoint->moveStartCtrlPtToAbsPos(absPtPos);
    newPoint->moveEndCtrlPtToAbsPos(absPtPos);

    return addPoint(newPoint, toPoint);
}

PathPoint *SinglePathAnimator::addPointRelPos(const QPointF &relPtPos,
                                        PathPoint *toPoint)
{
    PathPoint *newPoint = new PathPoint(this);
    newPoint->setRelativePos(relPtPos, false);
    newPoint->moveStartCtrlPtToRelPos(relPtPos);
    newPoint->moveEndCtrlPtToRelPos(relPtPos);

    return addPoint(newPoint, toPoint);
}

void SinglePathAnimator::appendToPointsList(PathPoint *point,
                                      const bool &saveUndoRedo) {
    mPoints.append(point);
    point->setParentPath(this);
    addChildAnimator(point->getPathPointAnimatorsPtr());
    //point->show();
    if(saveUndoRedo) {
        AppendToPointsListUndoRedo *undoRedo =
                new AppendToPointsListUndoRedo(point, this);
        addUndoRedo(undoRedo);
    }
    point->incNumberPointers();

    //schedulePathUpdate();

    updatePathPointIds();

    callUpdater();
}

void SinglePathAnimator::removeFromPointsList(PathPoint *point,
                                        const bool &saveUndoRedo) {
    mPoints.removeOne(point);
    removeChildAnimator(point->getPathPointAnimatorsPtr());
    //point->hide();
    //getParentCanvas()->removePointFromSelection(point);
    if(saveUndoRedo) {
        RemoveFromPointsListUndoRedo *undoRedo =
                new RemoveFromPointsListUndoRedo(point, this);
        addUndoRedo(undoRedo);
        if(mPoints.count() == 0) {
            point->decNumberPointers();
            mParentPathAnimator->removeSinglePathAnimator(this);
            return;
        }
    }
    point->decNumberPointers();

    //schedulePathUpdate();
    callUpdater();

    updatePathPointIds();
}

void SinglePathAnimator::removePoint(PathPoint *point) {
    PathPoint *prevPoint = point->getPreviousPoint();
    PathPoint *nextPoint = point->getNextPoint();

    if(prevPoint != NULL) {
        if(point->isSeparatePathPoint()) {
            replaceSeparatePathPoint(prevPoint);
        }
        prevPoint->setPointAsNext(nextPoint);
    } else if (nextPoint != NULL){
        replaceSeparatePathPoint(nextPoint);
        nextPoint->setPointAsPrevious(NULL);
    } else {
        replaceSeparatePathPoint(NULL);
    }
    removeFromPointsList(point);
}

void SinglePathAnimator::replaceSeparatePathPoint(PathPoint *newPoint,
                                                  const bool &saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new ChangeSinglePathFirstPoint(this,
                                                   mFirstPoint,
                                                   newPoint));
    }
    if(mFirstPoint != NULL) {
        mFirstPoint->setSeparatePathPoint(false);
    }
    mFirstPoint = newPoint;
    if(mFirstPoint != NULL) {
        mFirstPoint->setSeparatePathPoint(true);
        updatePathPointIds();
    }
}

void SinglePathAnimator::startAllPointsTransform()
{
    foreach(PathPoint *point, mPoints) {
        point->startTransform();
    }
}

void SinglePathAnimator::finishAllPointsTransform() {
    foreach(PathPoint *point, mPoints) {
        point->finishTransform();
    }
}

void SinglePathAnimator::duplicatePathPointsTo(
        SinglePathAnimator *target) {
    PathPoint *currPoint = mFirstPoint;
    PathPoint *lastAddedPoint = NULL;
    PathPoint *firstAddedPoint = NULL;
    while(true) {
        PathPoint *pointToAdd = new PathPoint(target);
        currPoint->makeDuplicate(pointToAdd);
        target->addPoint(pointToAdd, lastAddedPoint);
        PathPoint *nextPoint = currPoint->getNextPoint();
        if(nextPoint == mFirstPoint) {
            pointToAdd->connectToPoint(firstAddedPoint);
            break;
        }
        lastAddedPoint = pointToAdd;
        if(firstAddedPoint == NULL) {
            firstAddedPoint = pointToAdd;
        }
        if(nextPoint == NULL) break;
        currPoint = nextPoint;
    }
}
