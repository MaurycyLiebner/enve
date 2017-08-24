#include "singlevectorpathanimator.h"
#include "canvas.h"
#include "nodepoint.h"
#include "pointhelpers.h"
#include "edge.h"
#include "undoredo.h"
#include "Animators/pathanimator.h"

SingleVectorPathAnimator::SingleVectorPathAnimator(PathAnimator *parentPath) :
    SinglePathAnimator() {
    mParentPathAnimator = parentPath;
}

void SingleVectorPathAnimator::drawSelected(SkCanvas *canvas,
                                      const CanvasMode &currentCanvasMode,
                                      const qreal &invScale,
                                      const SkMatrix &combinedTransform) {

    if(currentCanvasMode == CanvasMode::MOVE_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            const QSharedPointer<NodePoint> &point = mPoints.at(i);
            point->drawSk(canvas, currentCanvasMode, invScale);
        }
    } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            const QSharedPointer<NodePoint> &point = mPoints.at(i);
            if(point->isEndPoint() || point->isSelected()) {
                point->drawSk(canvas, currentCanvasMode, invScale);
            }
        }
    }
}

//void SingleVectorPathAnimator::loadPointsFromSql(int boundingBoxId) {
//    QSqlQuery query;
//    QString queryStr = QString("SELECT id, isfirst, isendpoint, qpointfanimatorid "
//                               "FROM NodePoint WHERE boundingboxid = %1 "
//                               "ORDER BY id ASC").arg(boundingBoxId);
//    if(query.exec(queryStr) ) {
//        int idisfirst = query.record().indexOf("isfirst");
//        int idisendpoint = query.record().indexOf("isendpoint");
//        int idqpointfanimatorid = query.record().indexOf("qpointfanimatorid");
//        int idId = query.record().indexOf("id");
//        NodePoint *firstPoint = NULL;
//        NodePoint *lastPoint = NULL;
//        while(query.next()) {
//            int id = query.value(idId).toInt();
//            bool isfirst = query.value(idisfirst).toBool();
//            bool isendpoint = query.value(idisendpoint).toBool();
//            int qpointfanimatorid = query.value(idqpointfanimatorid).toInt();

//            NodePoint *newPoint = new NodePoint(this);
//            newPoint->loadFromSql(id, qpointfanimatorid);
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
//}

void SingleVectorPathAnimator::changeAllPointsParentPathTo(
        SingleVectorPathAnimator *path) {
    replaceSeparateNodePoint(NULL);
    QList<QSharedPointer<NodePoint> > allPoints = mPoints;
    Q_FOREACH(const QSharedPointer<NodePoint> &point, allPoints) {
        removeFromPointsList(point.data());
        path->appendToPointsList(point.data());
    }
    mPoints.clear();
}

void SingleVectorPathAnimator::deletePointAndApproximate(
                                NodePoint *pointToRemove) {
    NodePoint *nextPoint = pointToRemove->getNextPoint();
    NodePoint *prevPoint = pointToRemove->getPreviousPoint();
    if(nextPoint == NULL || prevPoint == NULL) return;

    nextPoint->setCtrlsMode(CtrlsMode::CTRLS_CORNER);
    prevPoint->setCtrlsMode(CtrlsMode::CTRLS_CORNER);

    QPointF absPos = pointToRemove->getAbsolutePos();

    pointToRemove->removeFromVectorPath();

    VectorPathEdge *newEdge = prevPoint->getNextEdge();
    newEdge->setPressedT(0.5);
    newEdge->makePassThrough(absPos);
}

void SingleVectorPathAnimator::selectAndAddContainedPointsToList(
                                const QRectF &absRect,
                                QList<MovablePoint *> *list) {
    Q_FOREACH(const QSharedPointer<NodePoint> &point, mPoints) {
        point->rectPointsSelection(absRect, list);
    }
}

NodePoint *SingleVectorPathAnimator::addPoint(NodePoint *pointToAdd,
                                              NodePoint *toPoint)
{
    if(toPoint == NULL) {
        replaceSeparateNodePoint(pointToAdd);
    } else {
        if(!toPoint->hasNextPoint()) {
            toPoint->setPointAsNext(pointToAdd);
        } else if(!toPoint->hasPreviousPoint() ) {
            replaceSeparateNodePoint(pointToAdd);
            toPoint->setPointAsPrevious(pointToAdd);
        }
    }

    appendToPointsList(pointToAdd);

    return pointToAdd;
}

NodePoint* SingleVectorPathAnimator::addPointAbsPos(
                                const QPointF &absPtPos,
                                NodePoint *toPoint) {
//    NodePoint *newPoint = new NodePoint(this);
//    newPoint->setAbsolutePos(absPtPos, false);
//    newPoint->moveStartCtrlPtToAbsPos(absPtPos);
//    newPoint->moveEndCtrlPtToAbsPos(absPtPos);

//    return addPoint(newPoint, toPoint);
}

NodePoint *SingleVectorPathAnimator::addPointRelPos(
                                const QPointF &relPtPos,
                                NodePoint *toPoint) {
//    NodePoint *newPoint = new NodePoint(this);
//    newPoint->setRelativePos(relPtPos, false);
//    newPoint->moveStartCtrlPtToRelPos(relPtPos);
//    newPoint->moveEndCtrlPtToRelPos(relPtPos);

//    return addPoint(newPoint, toPoint);
}

void SingleVectorPathAnimator::appendToPointsList(
                    NodePoint *point,
                    const bool &saveUndoRedo) {
//    mPoints.append(point->ref<NodePoint>());
//    point->setParentPath(this);
//    if(saveUndoRedo) {
//        AppendToPointsListUndoRedo *undoRedo =
//                new AppendToPointsListUndoRedo(point, this);
//        addUndoRedo(undoRedo);
//    }


//    prp_updateInfluenceRangeAfterChanged();
//    prp_callUpdater();

//    updateNodePointIds();
}

int SingleVectorPathAnimator::getChildPointIndex(NodePoint *child) {
    int index = -1;
    for(int i = 0; i < mPoints.count(); i++) {
        if(mPoints.at(i) == child) {
            index = i;
        }
    }
    return index;
}

void SingleVectorPathAnimator::removeFromPointsList(NodePoint *point,
                                              const bool &saveUndoRedo) {
//    if(saveUndoRedo) {
//        addUndoRedo(new RemoveFromPointsListUndoRedo(point, this));
//        if(mPoints.count() == 0) {
//            mParentPathAnimator->removeSinglePathAnimator(this);
//        }
//    }
//    mPoints.removeAt(getChildPointIndex(point));

//    prp_updateInfluenceRangeAfterChanged();
//    prp_callUpdater();

//    updateNodePointIds();
}

void SingleVectorPathAnimator::removePoint(NodePoint *point) {
    NodePoint *prevPoint = point->getPreviousPoint();
    NodePoint *nextPoint = point->getNextPoint();

    if(prevPoint != NULL) {
        if(point->isSeparateNodePoint()) {
            replaceSeparateNodePoint(prevPoint);
        }
        prevPoint->setPointAsNext(nextPoint);
    } else if (nextPoint != NULL){
        replaceSeparateNodePoint(nextPoint);
        nextPoint->setPointAsPrevious(NULL);
    } else {
        replaceSeparateNodePoint(NULL);
    }
    removeFromPointsList(point);
}

void SingleVectorPathAnimator::replaceSeparateNodePoint(
                                    NodePoint *newPoint,
                                    const bool &saveUndoRedo) {
//    if(saveUndoRedo) {
//        addUndoRedo(new ChangeSingleVectorPathFirstPoint(this,
//                                                   mFirstPoint,
//                                                   newPoint));
//    }
//    if(mFirstPoint != NULL) {
//        mFirstPoint->setSeparateNodePoint(false);
//    }
//    mFirstPoint = newPoint;
//    if(mFirstPoint != NULL) {
//        mFirstPoint->setSeparateNodePoint(true);
//        updateNodePointIds();
//    }
}

void SingleVectorPathAnimator::startAllPointsTransform() {
    Q_FOREACH(const QSharedPointer<NodePoint> &point, mPoints) {
        point->startTransform();
    }
}

void SingleVectorPathAnimator::finishAllPointsTransform() {
    Q_FOREACH(const QSharedPointer<NodePoint> &point, mPoints) {
        point->finishTransform();
    }
}

void SingleVectorPathAnimator::duplicateNodePointsTo(
        SingleVectorPathAnimator *target) {
//    NodePoint *currPoint = mFirstPoint;
//    NodePoint *lastAddedPoint = NULL;
//    NodePoint *firstAddedPoint = NULL;
//    while(true) {
//        NodePoint *pointToAdd = new NodePoint(target);
//        currPoint->makeDuplicate(pointToAdd);
//        target->addPoint(pointToAdd, lastAddedPoint);
//        NodePoint *nextPoint = currPoint->getNextPoint();
//        if(nextPoint == mFirstPoint) {
//            pointToAdd->connectToPoint(firstAddedPoint);
//            break;
//        }
//        lastAddedPoint = pointToAdd;
//        if(firstAddedPoint == NULL) {
//            firstAddedPoint = pointToAdd;
//        }
//        if(nextPoint == NULL) break;
//        currPoint = nextPoint;
//    }
}

VectorPathEdge *SingleVectorPathAnimator::getEdge(const QPointF &absPos,
                                                  const qreal &canvasScaleInv) {
    qreal pressedT;
    NodePoint *prevPoint = NULL;
    NodePoint *nextPoint = NULL;
    if(getTAndPointsForMouseEdgeInteraction(absPos, &pressedT,
                                            &prevPoint, &nextPoint,
                                            canvasScaleInv)) {
        if(pressedT > 0.0001 && pressedT < 0.9999 && prevPoint && nextPoint) {
            VectorPathEdge *edge = prevPoint->getNextEdge();
            edge->setPressedT(pressedT);
            return edge;
        } else {
            return NULL;
        }
    }
    return NULL;
}

MovablePoint *SingleVectorPathAnimator::getPointAtAbsPos(
                        const QPointF &absPtPos,
                        const CanvasMode &currentCanvasMode,
                        const qreal &canvasScaleInv) {
    Q_FOREACH(const QSharedPointer<NodePoint> &point, mPoints) {
        MovablePoint *pointToReturn =
                point->getPointAtAbsPos(absPtPos,
                                        currentCanvasMode,
                                        canvasScaleInv);
        if(pointToReturn == NULL) continue;
        return pointToReturn;
    }
    return NULL;
}

void SingleVectorPathAnimator::selectAllPoints(Canvas *canvas) {
    Q_FOREACH(const QSharedPointer<NodePoint> &point, mPoints) {
        canvas->addPointToSelection(point.data());
    }
}

void SingleVectorPathAnimator::saveToSql(QSqlQuery *query,
                                         const int &boundingBoxId) {
    //mFirstPoint->saveToSql(query, boundingBoxId);
}


void SingleVectorPathAnimator::updateNodePointIds() {
    if(mFirstPoint == NULL) return;
    int pointId = 0;
    NodePoint *nextPoint = mFirstPoint;
    while(true) {
        nextPoint->setPointId(pointId);
        pointId++;
        nextPoint = nextPoint->getNextPoint();
        if(nextPoint == NULL || nextPoint == mFirstPoint) break;
    }
}

NodePoint *SingleVectorPathAnimator::addPointRelPos(const QPointF &relPos,
                                        const QPointF &startRelPos,
                                        const QPointF &endRelPos,
                                        NodePoint *toPoint) {
    NodePoint *newPoint = addPointRelPos(relPos, toPoint);
    //newPoint->setCtrlsMode(CTRLS_SYMMETRIC);
    newPoint->setStartCtrlPtEnabled(true);
    newPoint->setEndCtrlPtEnabled(true);
    newPoint->moveStartCtrlPtToRelPos(startRelPos);
    newPoint->moveEndCtrlPtToRelPos(endRelPos);
    return newPoint;
}


SkPath SingleVectorPathAnimator::getPathAtRelFrame(const int &relFrame) {
    SkPath path = SkPath();

    NodePoint *point = mFirstPoint;
    NodePointValues lastPointValues;
    //lastPointValues = point->getPointValuesAtRelFrame(relFrame);
    path.moveTo(QPointFToSkPoint(lastPointValues.pointRelPos));
    while(true) {
        point = point->getNextPoint();
        if(point == NULL) break;
        NodePointValues pointValues;

        //pointValues = point->getPointValuesAtRelFrame(relFrame);

        path.cubicTo(QPointFToSkPoint(lastPointValues.endRelPos),
                     QPointFToSkPoint(pointValues.startRelPos),
                     QPointFToSkPoint(pointValues.pointRelPos));

        lastPointValues = pointValues;

        if(point == mFirstPoint) {
            path.close();
            break;
        }
    }

    return path;
}

bool SingleVectorPathAnimator::getTAndPointsForMouseEdgeInteraction(
                                          const QPointF &absPos,
                                          qreal *pressedT,
                                          NodePoint **prevPoint,
                                          NodePoint **nextPoint,
                                          const qreal &canvasScaleInv) {
    const QMatrix &combinedTransform =
            mParentPathAnimator->getParentBox()->getCombinedTransform();
    qreal xScaling = combinedTransform.map(
                        QLineF(0., 0., 1., 0.)).length();
    qreal yScaling = combinedTransform.map(
                        QLineF(0., 0., 0., 1.)).length();
    qreal maxDistX = 4./xScaling*canvasScaleInv;
    qreal maxDistY = 4./yScaling*canvasScaleInv;
    QPointF relPos = combinedTransform.inverted().map(absPos);

//    if(!mPath.intersects(distRect) ||
//        mPath.contains(distRect)) {
//        return false;
//    }

    QPointF nearestPos = relPos + QPointF(maxDistX, maxDistY)*2;
    qreal nearestT = 0.5;
    qreal nearestError = 1000000.;

    foreach(const QSharedPointer<NodePoint> &NodePoint, mPoints) {
        VectorPathEdge *edgeT = NodePoint->getNextEdge();
        if(edgeT == NULL) continue;
        QPointF posT;
        qreal tT;
        qreal errorT;
        edgeT->getNearestRelPosAndT(relPos,
                                    &posT,
                                    &tT,
                                    &errorT);
        if(errorT < nearestError) {
            nearestError = errorT;
            nearestT = tT;
            nearestPos = posT;
            *prevPoint = NodePoint.data();
        }
    }
    QPointF distT = nearestPos - relPos;
    if(qAbs(distT.x()) > maxDistX ||
       qAbs(distT.y()) > maxDistY) return false;

    *pressedT = nearestT;
    if(*prevPoint == NULL) return false;

    *nextPoint = (*prevPoint)->getNextPoint();
    if(*nextPoint == NULL) return false;

    return true;
}


NodePoint *SingleVectorPathAnimator::createNewPointOnLineNear(
                                    const QPointF &absPos,
                                    const bool &adjust,
                                    const qreal &canvasScaleInv) {
//    qreal pressedT;
//    NodePoint *prevPoint = NULL;
//    NodePoint *nextPoint = NULL;
//    if(getTAndPointsForMouseEdgeInteraction(absPos, &pressedT,
//                                            &prevPoint, &nextPoint,
//                                            canvasScaleInv)) {
//        if(pressedT > 0.0001 && pressedT < 0.9999) {
//            QPointF prevPointEnd = prevPoint->getEndCtrlPtValue();
//            QPointF nextPointStart = nextPoint->getStartCtrlPtValue();
//            QPointF newPointPos;
//            QPointF newPointStart;
//            QPointF newPointEnd;
//            VectorPathEdge::getNewRelPosForKnotInsertionAtT(
//                          prevPoint->getRelativePos(),
//                          &prevPointEnd,
//                          &nextPointStart,
//                          nextPoint->getRelativePos(),
//                          &newPointPos,
//                          &newPointStart,
//                          &newPointEnd,
//                          pressedT);

//            NodePoint *newPoint = new NodePoint(this);
//            newPoint->setRelativePos(newPointPos, false);

//            nextPoint->setPointAsPrevious(newPoint);
//            prevPoint->setPointAsNext(newPoint);

//            if(adjust) {
//                if(!prevPoint->isEndCtrlPtEnabled() &&
//                   !nextPoint->isStartCtrlPtEnabled()) {
//                    newPoint->setStartCtrlPtEnabled(false);
//                    newPoint->setEndCtrlPtEnabled(false);
//                } else {
//                    newPoint->setCtrlsMode(CtrlsMode::CTRLS_SMOOTH, false);
//                    newPoint->setStartCtrlPtEnabled(true);
//                    newPoint->moveStartCtrlPtToRelPos(newPointStart);
//                    newPoint->setEndCtrlPtEnabled(true);
//                    newPoint->moveEndCtrlPtToRelPos(newPointEnd);

//                    if(prevPoint->getCurrentCtrlsMode() ==
//                       CtrlsMode::CTRLS_SYMMETRIC &&
//                       prevPoint->isEndCtrlPtEnabled() &&
//                       prevPoint->isStartCtrlPtEnabled()) {
//                        prevPoint->setCtrlsMode(CtrlsMode::CTRLS_SMOOTH);
//                    }
//                    if(nextPoint->getCurrentCtrlsMode() ==
//                       CtrlsMode::CTRLS_SYMMETRIC &&
//                       nextPoint->isEndCtrlPtEnabled() &&
//                       nextPoint->isStartCtrlPtEnabled()) {
//                        nextPoint->setCtrlsMode(CtrlsMode::CTRLS_SMOOTH);
//                    }
//                    prevPoint->moveEndCtrlPtToRelPos(prevPointEnd);
//                    nextPoint->moveStartCtrlPtToRelPos(nextPointStart);
//                }
//            }

//            appendToPointsList(newPoint);

//            return newPoint;
//        }
//    }
    return NULL;
}

void SingleVectorPathAnimator::applyTransformToPoints(
        const QMatrix &transform) {
    Q_FOREACH(const QSharedPointer<NodePoint> &point, mPoints) {
        point->applyTransform(transform);
    }
}

void SingleVectorPathAnimator::disconnectPoints(NodePoint *point1,
                                          NodePoint *point2) {
//    if(point1->getNextPoint() != point2 &&
//       point1->getPreviousPoint() != point2) return;
//    if(point1->getPreviousPoint() == point2) {
//        NodePoint *point2T = point2;
//        point2 = point1;
//        point1 = point2T;
//    }
//    point1->disconnectFromPoint(point2);

//    if(mFirstPoint == point2) return;
//    bool stillConnected = false;
//    NodePoint *currPoint = point1;
//    NodePoint *prevPoint = point1->getPreviousPoint();
//    while(prevPoint != NULL) {
//        currPoint = prevPoint;
//        prevPoint = currPoint->getPreviousPoint();
//    }
//    stillConnected = currPoint == point2;
//    if(stillConnected) {
//        replaceSeparateNodePoint(point2);
//    } else {
//        SingleVectorPathAnimator *newPath = new SingleVectorPathAnimator(
//                                                mParentPathAnimator);
//        NodePoint *currPt = point2;
//        while(true) {
//            removeFromPointsList(currPt);
//            newPath->appendToPointsList(currPt);
//            currPt = currPt->getNextPoint();
//            if(currPt == point2 || currPt == NULL) break;
//        }
//        newPath->replaceSeparateNodePoint(point2);

//        newPath->updateNodePointIds();
//        mParentPathAnimator->addSinglePathAnimator(newPath);
//    }

//    updateNodePointIds();
//    prp_callUpdater();
}

void SingleVectorPathAnimator::connectPoints(NodePoint *point1,
                                       NodePoint *point2) {
//    NodePoint *point1FirstPoint = point1;
//    NodePoint *point2FirstPoint = point2;
//    while(point1FirstPoint->getNextPoint() != NULL) {
//        point1FirstPoint = point1FirstPoint->getNextPoint();
//    }
//    while(point2FirstPoint->getNextPoint() != NULL) {
//        point2FirstPoint = point2FirstPoint->getNextPoint();
//    }
//    if(point1FirstPoint == point2FirstPoint) {
//        point1->connectToPoint(point2);
//        return;
//    }

//    SingleVectorPathAnimator *point1ParentPath = point1->getParentPath();
//    SingleVectorPathAnimator *point2ParentPath = point2->getParentPath();
//    if(point1->isSeparateNodePoint() &&
//       point2->isSeparateNodePoint()) {
//        point1->reversePointsDirectionStartingFromThis();
//        point1ParentPath->changeAllPointsParentPathTo(point2ParentPath);
//        mParentPathAnimator->removeSinglePathAnimator(point1ParentPath);

//        point2ParentPath->replaceSeparateNodePoint(NULL);

//        point1->connectToPoint(point2);
//        NodePoint *firstPtCandidate = point1->getConnectedSeparateNodePoint();
//        point2ParentPath->replaceSeparateNodePoint(firstPtCandidate);
//        point2ParentPath->updateNodePointIds();
//    } else if(point1->isSeparateNodePoint()) {
//        point1ParentPath->changeAllPointsParentPathTo(point2ParentPath);
//        mParentPathAnimator->removeSinglePathAnimator(point1ParentPath);
//        point1->connectToPoint(point2);
//        point2ParentPath->updateNodePointIds();
//    } else if(point2->isSeparateNodePoint()) {
//        point2ParentPath->changeAllPointsParentPathTo(point1ParentPath);
//        mParentPathAnimator->removeSinglePathAnimator(point2ParentPath);
//        point1->connectToPoint(point2);
//        point1ParentPath->updateNodePointIds();
//    } else {
//        NodePoint *point1ConnectedFirst =
//                point1->getConnectedSeparateNodePoint();

//        point1ConnectedFirst->reversePointsDirectionStartingFromThis();

//        point1ParentPath->changeAllPointsParentPathTo(
//                    point2ParentPath);
//        mParentPathAnimator->removeSinglePathAnimator(
//                    point1ParentPath);

//        point1->connectToPoint(point2);
//        point2ParentPath->updateNodePointIds();
//    }
}
