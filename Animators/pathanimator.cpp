#include "Animators/pathanimator.h"
#include "singlepathanimator.h"
#include "pathpoint.h"
#include "undoredo.h"
#include "edge.h"
#include "skiaincludes.h"
#include <QSqlRecord>

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

}

void PathAnimator::setParentBox(BoundingBox *parent) {
    mParentBox = parent;
}

VectorPathEdge *PathAnimator::getEgde(const QPointF &absPos,
                                      const qreal &canvasScaleInv) {
    Q_FOREACH(SinglePathAnimator *path, mSinglePaths) {
        VectorPathEdge *edge = path->getEgde(absPos, canvasScaleInv);
        if(edge == NULL) continue;
        return edge;
    }
    return NULL;
}

QPointF PathAnimator::getRelCenterPosition() {
//    QPointF posSum = QPointF(0., 0.);
//    int count = mPoints.length();
//    if(count == 0) return posSum;
//    Q_FOREACH(PathPoint *point, mPoints) {
//        posSum += point->getRelativePos();
//    }
//    return posSum/count;
    return mPath.controlPointRect().center();
}

void PathAnimator::addSinglePathAnimator(SinglePathAnimator *path,
                                         const bool &saveUndoRedo) {
    mSinglePaths << path;
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
        ca_removeChildAnimator(path);
    }
}

bool PathAnimator::SWT_isPathAnimator() { return true; }

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

void PathAnimator::updatePath() {
    mPath = QPainterPath();
    mSkPath = SkPath();

    Q_FOREACH(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->updatePath();
        mPath.addPath(singlePath->getCurrentPath());
        singlePath->updateSkPath();
        mSkPath.addPath(singlePath->getCurrentSkPath());
    }
}

PathPoint *PathAnimator::createNewPointOnLineNear(const QPointF &absPos,
                                                  const bool &adjust,
                                                  const qreal &canvasScaleInv) {
    Q_FOREACH(SinglePathAnimator *singlePath, mSinglePaths) {
        PathPoint *pt = singlePath->createNewPointOnLineNear(absPos,
                                                             adjust,
                                                             canvasScaleInv);
        if(pt == NULL) continue;
        return pt;
    }
    return NULL;
}

void PathAnimator::updateAfterFrameChanged(const int &currentFrame) {
    Q_FOREACH(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->updateAfterFrameChanged(currentFrame);
    }
}

qreal PathAnimator::findPercentForPoint(const QPointF &point,
                                        PathPoint **prevPoint,
                                        qreal *error) {
    Q_FOREACH(SinglePathAnimator *singlePath, mSinglePaths) {
        const qreal &val =
                singlePath->findPercentForPoint(point, prevPoint, error);
        if(*prevPoint == NULL) continue;
        return val;
    }
    return 0.;
}

void PathAnimator::applyTransformToPoints(const QMatrix &transform) {
    Q_FOREACH(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->applyTransformToPoints(transform);
    }
}

MovablePoint *PathAnimator::qra_getPointAt(
                                    const QPointF &absPtPos,
                                    const CanvasMode &currentCanvasMode,
                                    const qreal &canvasScaleInv) {
    Q_FOREACH(SinglePathAnimator *sepAnim, mSinglePaths) {
        MovablePoint *pt = sepAnim->qra_getPointAt(absPtPos,
                                                   currentCanvasMode,
                                                   canvasScaleInv);
        if(pt == NULL) continue;
        return pt;
    }

    return NULL;
}

const QPainterPath &PathAnimator::getCurrentPath() {
    return mPath;
}

const SkPath &PathAnimator::getCurrentSkPath() {
    return mSkPath;
}

void PathAnimator::drawSelected(QPainter *p,
                                const CanvasMode &currentCanvasMode,
                                const QMatrix &combinedTransform) {
    Q_FOREACH(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->drawSelected(p, currentCanvasMode, combinedTransform);
    }
}

void PathAnimator::drawSelected(SkCanvas *canvas,
                                const CanvasMode &currentCanvasMode,
                                const SkScalar &invScale,
                                const SkMatrix &combinedTransform) {
    Q_FOREACH(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->drawSelected(canvas,
                                 currentCanvasMode,
                                 invScale,
                                 combinedTransform);
    }
}

void PathAnimator::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<MovablePoint *> *list) {
    Q_FOREACH(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->selectAndAddContainedPointsToList(absRect,
                                                      list);
    }
}

BoundingBox *PathAnimator::getParentBox() {
    return mParentBox;
}

int PathAnimator::prp_saveToSql(QSqlQuery *query,
                                const int &boundingBoxId) {
    Q_FOREACH(SinglePathAnimator *singlePath, mSinglePaths) {
        singlePath->savePointsToSql(query, boundingBoxId);
    }
    return 0;
}

void PathAnimator::makeDuplicate(Property *property) {
    duplicatePathsTo((PathAnimator*)property);
}

Property *PathAnimator::makeDuplicate() {
    PathAnimator *newAnimator = new PathAnimator();
    makeDuplicate(newAnimator);
    return newAnimator;
}

void PathAnimator::prp_loadFromSql(const int &boundingBoxId) {
    QSqlQuery query;
    QString queryStr = QString("SELECT id, isfirst, isendpoint, qpointfanimatorid "
                               "FROM pathpoint WHERE boundingboxid = %1 "
                               "ORDER BY id ASC").arg(boundingBoxId);
    if(query.exec(queryStr) ) {
        int idisfirst = query.record().indexOf("isfirst");
        int idisendpoint = query.record().indexOf("isendpoint");
        int idqpointfanimatorid = query.record().indexOf("qpointfanimatorid");
        PathPoint *firstPoint = NULL;
        PathPoint *lastPoint = NULL;
        SinglePathAnimator *singlePathAnimator = NULL;
        while(query.next()) {
            bool isfirst = query.value(idisfirst).toBool();
            bool isendpoint = query.value(idisendpoint).toBool();
            int qpointfanimatorid = query.value(idqpointfanimatorid).toInt();

            PathPoint *newPoint;
            if(isfirst) {
                if(lastPoint != NULL && firstPoint != NULL) {
                    lastPoint->setPointAsNext(firstPoint, false);
                }
                lastPoint = NULL;
                singlePathAnimator = new SinglePathAnimator(this);
                addSinglePathAnimator(singlePathAnimator);
                newPoint = new PathPoint(singlePathAnimator);
                newPoint->prp_loadFromSql(qpointfanimatorid);

                if(isendpoint) {
                    firstPoint = NULL;
                } else {
                    firstPoint = newPoint;
                }
            } else {
                newPoint = new PathPoint(singlePathAnimator);
                newPoint->prp_loadFromSql(qpointfanimatorid);
            }
            singlePathAnimator->addPoint(newPoint, lastPoint);
            lastPoint = newPoint;
        }
        if(lastPoint != NULL && firstPoint != NULL) {
            lastPoint->setPointAsNext(firstPoint, false);
        }
    } else {
        qDebug() << "Could not load points for vectorpath with id " << boundingBoxId;
    }
}

void PathAnimator::duplicatePathsTo(
        PathAnimator *target) {
    Q_FOREACH(SinglePathAnimator *sepPoint, mSinglePaths) {
        SinglePathAnimator *copy = new SinglePathAnimator(target);
        sepPoint->duplicatePathPointsTo(copy);
        target->addSinglePathAnimator(copy);
    }
}
