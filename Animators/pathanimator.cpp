#include "Animators/pathanimator.h"
#include "singlepathanimator.h"
#include "pathpoint.h"
#include "undoredo.h"
#include "edge.h"

PathAnimator::PathAnimator() :
    ComplexAnimator() {
    prp_setName("path");
}

PathAnimator::PathAnimator(BoundingBox *parentBox) :
    PathAnimator() {
    setParentBox(parentBox);
    prp_setName("path");
}

PathAnimator::~PathAnimator() {
    foreach(SinglePathAnimator *path, mSinglePaths) {
        path->decNumberPointers();
    }
}

void PathAnimator::setParentBox(BoundingBox *parent) {
    mParentBox = parent;
}

Edge *PathAnimator::getEgde(QPointF absPos) {
    foreach(SinglePathAnimator *path, mSinglePaths) {
        Edge *edge = path->getEgde(absPos);
        if(edge == NULL) continue;
        return edge;
    }
    return NULL;
}

QPointF PathAnimator::getRelCenterPosition() {
//    QPointF posSum = QPointF(0., 0.);
//    int count = mPoints.length();
//    if(count == 0) return posSum;
//    foreach(PathPoint *point, mPoints) {
//        posSum += point->getRelativePos();
//    }
//    return posSum/count;
    return mPath.controlPointRect().center();
}

void PathAnimator::addSinglePathAnimator(SinglePathAnimator *path,
                                         const bool &saveUndoRedo) {
    mSinglePaths << path;
    path->incNumberPointers();
    ca_addChildAnimator(path);
    if(saveUndoRedo) {
        addUndoRedo(new AddSinglePathAnimatorUndoRedo(this, path));
    }
}

void PathAnimator::removeSinglePathAnimator(SinglePathAnimator *path,
                                            const bool &saveUndoRedo) {
    if(mSinglePaths.removeOne(path) ) {
        if(saveUndoRedo) {
            addUndoRedo(new RemoveSinglePathAnimatorUndoRedo(this, path));
        }
        path->decNumberPointers();
        ca_removeChildAnimator(path);
    }
}

void PathAnimator::loadPathFromQPainterPath(const QPainterPath &path) {
    PathPoint *firstPoint = NULL;
    PathPoint *lastPoint = NULL;
    bool firstOther = true;
    QPointF startCtrlPoint;
    SinglePathAnimator *singlePathAnimator = NULL;

    for(int i = 0; i < path.elementCount(); i++) {
        const QPainterPath::Element &elem = path.elementAt(i);

        if (elem.isMoveTo()) { // move
            if(singlePathAnimator != NULL) {
                addSinglePathAnimator(singlePathAnimator);
            }
            singlePathAnimator = new SinglePathAnimator(this);
            lastPoint = singlePathAnimator->addPointRelPos(
                                    QPointF(elem.x, elem.y), NULL);
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
                lastPoint = singlePathAnimator->
                        addPointRelPos(QPointF(elem.x, elem.y),
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
                    lastPoint = singlePathAnimator->
                            addPointRelPos(QPointF(elem.x, elem.y),
                                               lastPoint);
                }
                lastPoint->setStartCtrlPtEnabled(true);
                lastPoint->moveStartCtrlPtToRelPos(startCtrlPoint);
            }
            firstOther = !firstOther;
        }
    }
    if(singlePathAnimator != NULL) {
        addSinglePathAnimator(singlePathAnimator);
    }
}

void PathAnimator::updatePath()
{
    mPath = QPainterPath();

    foreach(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->updatePath();
        mPath.addPath(singlePath->getCurrentPath());
    }
}

PathPoint *PathAnimator::createNewPointOnLineNear(const QPointF &absPos,
                                                  const bool &adjust) {
    foreach(SinglePathAnimator *singlePath, mSinglePaths) {
        PathPoint *pt = singlePath->createNewPointOnLineNear(absPos, adjust);
        if(pt == NULL) continue;
        return pt;
    }
    return NULL;
}

void PathAnimator::updateAfterFrameChanged(const int &currentFrame) {
    foreach(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->updateAfterFrameChanged(currentFrame);
    }
}

qreal PathAnimator::findPercentForPoint(const QPointF &point,
                                        PathPoint **prevPoint,
                                        qreal *error) {
    foreach(SinglePathAnimator *singlePath, mSinglePaths) {
        const qreal &val =
                singlePath->findPercentForPoint(point, prevPoint, error);
        if(*prevPoint == NULL) continue;
        return val;
    }
    return 0.;
}

void PathAnimator::applyTransformToPoints(const QMatrix &transform) {
    foreach(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->applyTransformToPoints(transform);
    }
}

MovablePoint *PathAnimator::qra_getPointAt(const QPointF &absPtPos,
                                       const CanvasMode &currentCanvasMode)
{
    foreach(SinglePathAnimator *sepAnim, mSinglePaths) {
        MovablePoint *pt = sepAnim->qra_getPointAt(absPtPos, currentCanvasMode);
        if(pt == NULL) continue;
        return pt;
    }

    return NULL;
}

void PathAnimator::drawSelected(QPainter *p,
                                const CanvasMode &currentCanvasMode,
                                const QMatrix &combinedTransform) {
    foreach(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->drawSelected(p, currentCanvasMode, combinedTransform);
    }
}

void PathAnimator::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<MovablePoint *> *list) {
    foreach(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->selectAndAddContainedPointsToList(absRect,
                                                      list);
    }
}

void PathAnimator::savePointsToSql(QSqlQuery *query,
                                   const int &boundingBoxId) {
    foreach(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->savePointsToSql(query, boundingBoxId);
    }
}

void PathAnimator::loadPointsFromSql(int boundingBoxId) {
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

void PathAnimator::duplicatePathsTo(
        PathAnimator *target) {
    foreach(SinglePathAnimator *sepPoint, mSinglePaths) {
        SinglePathAnimator *copy = new SinglePathAnimator(target);
        sepPoint->duplicatePathPointsTo(copy);
        target->addSinglePathAnimator(copy);
    }
}
