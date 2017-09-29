#include "Animators/pathanimator.h"
#include "PathAnimators/vectorpathanimator.h"
#include "nodepoint.h"
#include "undoredo.h"
#include "edge.h"
#include "skqtconversions.h"
#include "canvas.h"

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

VectorPathEdge *PathAnimator::getEdge(const QPointF &absPos,
                                      const qreal &canvasScaleInv) {
    Q_FOREACH(VectorPathAnimator *path, mSinglePaths) {
        VectorPathEdge *edge = path->getEdge(absPos, canvasScaleInv);
        if(edge == NULL) continue;
        return edge;
    }
    return NULL;
}

void PathAnimator::addSinglePathAnimator(VectorPathAnimator *path,
                                         const bool &saveUndoRedo) {
    mSinglePaths << path;
    ca_addChildAnimator(path);
    if(saveUndoRedo) {
        addUndoRedo(new AddSinglePathAnimatorUndoRedo(this, path));
    }
}

void PathAnimator::removeSinglePathAnimator(VectorPathAnimator *path,
                                            const bool &saveUndoRedo) {
    if(mSinglePaths.removeOne(path) ) {
        if(saveUndoRedo) {
            addUndoRedo(new RemoveSinglePathAnimatorUndoRedo(this, path));
        }
        ca_removeChildAnimator(path);
    }
}

bool PathAnimator::SWT_isPathAnimator() { return true; }
#include "AddInclude/SkGeometry.h"
void PathAnimator::loadPathFromSkPath(const SkPath &path) {
    NodePoint *firstPoint = NULL;
    NodePoint *lastPoint = NULL;

    VectorPathAnimator *singlePathAnimator = NULL;

    SkPath::RawIter iter = SkPath::RawIter(path);

    SkPoint pts[4];
    int verbId = 0;

    // for converting conics to quads
    SkAutoConicToQuads conicToQuads;
    int quadsCount = 0;
    int quadId = 0;
    SkPoint *ptsT = NULL;

    SkPath::Verb verbT = iter.next(pts);
    for(;;) {
        switch(verbT) {
            case SkPath::kMove_Verb: {
                SkPoint pt = pts[0];
                if(singlePathAnimator != NULL) {
                    addSinglePathAnimator(singlePathAnimator);
                }
                singlePathAnimator = new VectorPathAnimator(this);
                lastPoint = singlePathAnimator->addNodeRelPos(
                                        SkPointToQPointF(pt),
                            NULL);
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
                            addNodeRelPos(SkPointToQPointF(pt),
                                          lastPoint);
                }
            }
                break;
            case SkPath::kConic_Verb: {
                ptsT = const_cast<SkPoint*>(conicToQuads.computeQuads(
                            pts, iter.conicWeight(), 2.f));
                quadsCount = conicToQuads.countQuads();
                quadId = 0;
            }
                break;
            case SkPath::kQuad_Verb: {
                SkPoint ctrlPtT = pts[1];
                pts[1] = pts[0] + (ctrlPtT - pts[0])*0.66667f;
                pts[3] = pts[2];
                pts[2] = pts[3] + (ctrlPtT - pts[3])*0.66667f;
                verbT = SkPath::kCubic_Verb;
                continue;
            }
                break;
            case SkPath::kCubic_Verb: {
                SkPoint endPt = pts[1];
                SkPoint startPt = pts[2];
                SkPoint targetPt = pts[3];
                lastPoint->setEndCtrlPtEnabled(true);
                lastPoint->moveEndCtrlPtToRelPos(
                            SkPointToQPointF(endPt));

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
                    singlePathAnimator->setPathClosed(true);
                } else {
                    lastPoint = singlePathAnimator->
                            addNodeRelPos(SkPointToQPointF(targetPt),
                                          lastPoint);
                }
                lastPoint->setStartCtrlPtEnabled(true);
                lastPoint->moveStartCtrlPtToRelPos(
                            SkPointToQPointF(startPt));
            }
                break;
            case SkPath::kClose_Verb:
                lastPoint->connectToPoint(firstPoint);
                lastPoint = firstPoint;
                singlePathAnimator->setPathClosed(true);
                break;
            case SkPath::kDone_Verb:
                goto DONE;
                break;
        }
        if(quadsCount > 0) {
            int firstPtId = quadId*2;
            pts[0] = ptsT[firstPtId];
            pts[1] = ptsT[firstPtId + 1];
            pts[2] = ptsT[firstPtId + 2];
            verbT = SkPath::kQuad_Verb;
            quadId++;
            quadsCount--;
        } else {
            verbT = iter.next(pts);
            verbId++;
        }
    }
DONE:
    if(singlePathAnimator != NULL) {
        addSinglePathAnimator(singlePathAnimator);
    }
}

SkPath PathAnimator::getPathAtRelFrame(const int &relFrame) {
    SkPath path = SkPath();

    Q_FOREACH(VectorPathAnimator *singlePath, mSinglePaths) {
        path.addPath(singlePath->getPathAtRelFrame(relFrame));
    }

    return path;
}

void PathAnimator::selectAllPoints(Canvas *canvas) {
    Q_FOREACH(VectorPathAnimator *singlePath, mSinglePaths) {
        singlePath->selectAllPoints(canvas);
    }
}

NodePoint *PathAnimator::createNewPointOnLineNear(const QPointF &absPos,
                                                  const bool &adjust,
                                                  const qreal &canvasScaleInv) {
    Q_FOREACH(VectorPathAnimator *singlePath, mSinglePaths) {
        NodePoint *pt = singlePath->createNewPointOnLineNear(absPos,
                                                             adjust,
                                                             canvasScaleInv);
        if(pt == NULL) continue;
        return pt;
    }
    return NULL;
}

void PathAnimator::applyTransformToPoints(const QMatrix &transform) {
    Q_FOREACH(VectorPathAnimator *singlePath, mSinglePaths) {
        singlePath->applyTransformToPoints(transform);
    }
}

MovablePoint *PathAnimator::getPointAtAbsPos(
                                    const QPointF &absPtPos,
                                    const CanvasMode &currentCanvasMode,
                                    const qreal &canvasScaleInv) {
    Q_FOREACH(VectorPathAnimator *sepAnim, mSinglePaths) {
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
    Q_FOREACH(VectorPathAnimator *singlePath, mSinglePaths) {
        singlePath->drawSelected(canvas,
                                 currentCanvasMode,
                                 invScale,
                                 combinedTransform);
    }
}

void PathAnimator::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<MovablePoint *> *list) {
    Q_FOREACH(VectorPathAnimator *singlePath, mSinglePaths) {
        singlePath->selectAndAddContainedPointsToList(absRect,
                                                      list);
    }
}

BoundingBox *PathAnimator::getParentBox() {
    return mParentBox;
}

void PathAnimator::makeDuplicate(Property *property) {
    duplicatePathsTo((PathAnimator*)property);
}

Property *PathAnimator::makeDuplicate() {
    PathAnimator *newAnimator = new PathAnimator();
    makeDuplicate(newAnimator);
    return newAnimator;
}

void PathAnimator::duplicatePathsTo(PathAnimator *target) {
    Q_FOREACH(VectorPathAnimator *path, mSinglePaths) {
        VectorPathAnimator *duplicate = path->makeDuplicate();
        duplicate->setParentPath(target);
        target->addSinglePathAnimator(duplicate);
    }
}
