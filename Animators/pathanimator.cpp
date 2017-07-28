#include "Animators/pathanimator.h"
#include "PathAnimators/singlevectorpathanimator.h"
#include "pathpoint.h"
#include "undoredo.h"
#include "edge.h"
#include "skqtconversions.h"
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
    Q_FOREACH(SingleVectorPathAnimator *path, mSinglePaths) {
        VectorPathEdge *edge = path->getEgde(absPos, canvasScaleInv);
        if(edge == NULL) continue;
        return edge;
    }
    return NULL;
}

void PathAnimator::addSinglePathAnimator(SingleVectorPathAnimator *path,
                                         const bool &saveUndoRedo) {
    mSinglePaths << path;
    ca_addChildAnimator(path);
    if(saveUndoRedo) {
        addUndoRedo(new AddSinglePathAnimatorUndoRedo(this, path));
    }
}

void PathAnimator::removeSinglePathAnimator(SingleVectorPathAnimator *path,
                                            const bool &saveUndoRedo) {
    if(mSinglePaths.removeOne(path) ) {
        if(saveUndoRedo) {
            addUndoRedo(new RemoveSinglePathAnimatorUndoRedo(this, path));
        }
        ca_removeChildAnimator(path);
    }
}

bool PathAnimator::SWT_isPathAnimator() { return true; }

void PathAnimator::loadPathFromSkPath(const SkPath &path) {
    PathPoint *firstPoint = NULL;
    PathPoint *lastPoint = NULL;

    SingleVectorPathAnimator *singlePathAnimator = NULL;

    SkPath::RawIter iter = SkPath::RawIter(path);

    SkPoint pts[4];
    int verbId = 0;
    for(;;) {
        switch(iter.next(pts)) {
            case SkPath::kMove_Verb: {
                SkPoint pt = pts[0];
                if(singlePathAnimator != NULL) {
                    addSinglePathAnimator(singlePathAnimator);
                }
                singlePathAnimator = new SingleVectorPathAnimator(this);
                lastPoint = singlePathAnimator->addPointRelPos(
                                        SkPointToQPointF(pt), NULL);
                firstPoint = lastPoint;
            }
                break;
            case SkPath::kLine_Verb: {
                SkPoint pt = pts[1];
                bool sameAsFirstPoint = SkPointToQPointF(pt) ==
                                            firstPoint->getRelativePos();
                bool connectOnly = false;
                if(sameAsFirstPoint) {
                    if(path.countVerbs() > verbId + 1) {
                        connectOnly = iter.peek() == SkPath::kMove_Verb;
                    } else {
                        connectOnly = true;
                    }
                }
                if(connectOnly) {
                    lastPoint->connectToPoint(firstPoint);
                    lastPoint = firstPoint;
                } else {
                    lastPoint = singlePathAnimator->
                            addPointRelPos(SkPointToQPointF(pt), lastPoint);
                }
            }
                break;
            case SkPath::kCubic_Verb: {
                SkPoint endPt = pts[1];
                SkPoint startPt = pts[2];
                SkPoint targetPt = pts[3];
                lastPoint->setEndCtrlPtEnabled(true);
                lastPoint->moveEndCtrlPtToRelPos(SkPointToQPointF(endPt));

                bool sameAsFirstPoint = SkPointToQPointF(targetPt) ==
                                            firstPoint->getRelativePos();
                bool connectOnly = false;
                if(sameAsFirstPoint) {
                    if(path.countVerbs() > verbId + 1) {
                        connectOnly = iter.peek() == SkPath::kMove_Verb;
                    } else {
                        connectOnly = true;
                    }
                }
                if(connectOnly) {
                    lastPoint->connectToPoint(firstPoint);
                    lastPoint = firstPoint;
                } else {
                    lastPoint = singlePathAnimator->
                            addPointRelPos(SkPointToQPointF(targetPt),
                                               lastPoint);
                }
                lastPoint->setStartCtrlPtEnabled(true);
                lastPoint->moveStartCtrlPtToRelPos(SkPointToQPointF(startPt));
            }
                break;
            case SkPath::kClose_Verb:
                lastPoint->connectToPoint(firstPoint);
                lastPoint = firstPoint;
                break;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
            case SkPath::kDone_Verb:
                goto DONE;
                break;
        }
        verbId++;
    }
DONE:
    if(singlePathAnimator != NULL) {
        addSinglePathAnimator(singlePathAnimator);
    }
}

SkPath PathAnimator::getPathAtRelFrame(const int &relFrame) {
    SkPath path = SkPath();

    Q_FOREACH(SingleVectorPathAnimator *singlePath, mSinglePaths) {
        path.addPath(singlePath->getPathAtRelFrame(relFrame));
    }

    return path;
}

PathPoint *PathAnimator::createNewPointOnLineNear(const QPointF &absPos,
                                                  const bool &adjust,
                                                  const qreal &canvasScaleInv) {
    Q_FOREACH(SingleVectorPathAnimator *singlePath, mSinglePaths) {
        PathPoint *pt = singlePath->createNewPointOnLineNear(absPos,
                                                             adjust,
                                                             canvasScaleInv);
        if(pt == NULL) continue;
        return pt;
    }
    return NULL;
}

void PathAnimator::applyTransformToPoints(const QMatrix &transform) {
    Q_FOREACH(SingleVectorPathAnimator *singlePath, mSinglePaths) {
        singlePath->applyTransformToPoints(transform);
    }
}

MovablePoint *PathAnimator::getPointAtAbsPos(
                                    const QPointF &absPtPos,
                                    const CanvasMode &currentCanvasMode,
                                    const qreal &canvasScaleInv) {
    Q_FOREACH(SingleVectorPathAnimator *sepAnim, mSinglePaths) {
        MovablePoint *pt = sepAnim->getPointAtAbsPos(absPtPos,
                                                     currentCanvasMode,
                                                     canvasScaleInv);
        if(pt == NULL) continue;
        return pt;
    }

    return NULL;
}

void PathAnimator::drawSelected(SkCanvas *canvas,
                                const CanvasMode &currentCanvasMode,
                                const SkScalar &invScale,
                                const SkMatrix &combinedTransform) {
    Q_FOREACH(SingleVectorPathAnimator *singlePath, mSinglePaths) {
        singlePath->drawSelected(canvas,
                                 currentCanvasMode,
                                 invScale,
                                 combinedTransform);
    }
}

void PathAnimator::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<MovablePoint *> *list) {
    Q_FOREACH(SingleVectorPathAnimator *singlePath, mSinglePaths) {
        singlePath->selectAndAddContainedPointsToList(absRect,
                                                      list);
    }
}

BoundingBox *PathAnimator::getParentBox() {
    return mParentBox;
}

int PathAnimator::saveToSql(QSqlQuery *query,
                            const int &boundingBoxId) {
    Q_FOREACH(SingleVectorPathAnimator *singlePath, mSinglePaths) {
        singlePath->saveToSql(query, boundingBoxId);
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

void PathAnimator::loadFromSql(const int &boundingBoxId) {
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
        SingleVectorPathAnimator *singlePathAnimator = NULL;
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
                singlePathAnimator = new SingleVectorPathAnimator(this);
                addSinglePathAnimator(singlePathAnimator);
                newPoint = new PathPoint(singlePathAnimator);
                newPoint->loadFromSql(qpointfanimatorid);

                if(isendpoint) {
                    firstPoint = NULL;
                } else {
                    firstPoint = newPoint;
                }
            } else {
                newPoint = new PathPoint(singlePathAnimator);
                newPoint->loadFromSql(qpointfanimatorid);
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

void PathAnimator::duplicatePathsTo(PathAnimator *target) {
    Q_FOREACH(SingleVectorPathAnimator *path, mSinglePaths) {
        SingleVectorPathAnimator *duplicate = path->makeDuplicate();
        duplicate->setParentPath(target);
        target->addSinglePathAnimator(duplicate);
    }
}
