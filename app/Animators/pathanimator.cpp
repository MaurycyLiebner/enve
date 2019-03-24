#include "Animators/pathanimator.h"
#include "PathAnimators/vectorpathanimator.h"
#include "MovablePoints/nodepoint.h"
#include "undoredo.h"
#include "edge.h"
#include "skia/skqtconversions.h"
#include "canvas.h"

PathAnimator::PathAnimator() : ComplexAnimator("path") {}

PathAnimator::PathAnimator(BoundingBox *parentBox) :
    PathAnimator() {
    setParentBox(parentBox);
}

void PathAnimator::setParentBox(BoundingBox *parent) {
    mParentBox = parent;
}

VectorPathEdge *PathAnimator::getEdge(const QPointF &absPos,
                                      const qreal &canvasScaleInv) {
    for(const auto& path : mSinglePaths) {
        VectorPathEdge *edge = path->getEdge(absPos, canvasScaleInv);
        if(!edge) continue;
        return edge;
    }
    return nullptr;
}

void PathAnimator::addSinglePathAnimator(const qsptr<VectorPathAnimator>& path) {
    mSinglePaths << path;
    ca_addChildAnimator(path);
}

void PathAnimator::removeSinglePathAnimator_k(const qsptr<VectorPathAnimator>& path) {
    if(mSinglePaths.removeOne(path) ) {
        ca_removeChildAnimator(path);
        if(mSinglePaths.isEmpty()) mParentBox->removeFromParent_k();
    }
}

bool PathAnimator::SWT_isPathAnimator() const { return true; }

void PathAnimator::loadPathFromSkPath(const SkPath &path) {
    NodePoint *firstPoint = nullptr;
    NodePoint *lastPoint = nullptr;

    qsptr<VectorPathAnimator> singlePathAnimator;

    SkPath::RawIter iter = SkPath::RawIter(path);

    SkPoint pts[4];
    int verbId = 0;

    // for converting conics to quads
    SkAutoConicToQuads conicToQuads;
    int quadsCount = 0;
    int quadId = 0;
    SkPoint *ptsT = nullptr;

    SkPath::Verb verbT = iter.next(pts);
    for(;;) {
        switch(verbT) {
            case SkPath::kMove_Verb: {
                SkPoint pt = pts[0];
                if(singlePathAnimator) {
                    addSinglePathAnimator(singlePathAnimator);
                }
                singlePathAnimator = SPtrCreate(VectorPathAnimator)(this);
                lastPoint = singlePathAnimator->addNodeRelPos(
                                        toQPointF(pt),
                            nullptr);
                firstPoint = lastPoint;
            }
                break;
            case SkPath::kLine_Verb: {
                SkPoint pt = pts[1];
                bool sameAsFirstPoint = pointToLen(toQPointF(pt) -
                                        firstPoint->getRelativePos()) < 0.1;

                bool connectOnly = false;
                if(sameAsFirstPoint) {
                    if(path.countVerbs() > verbId + 1) {
                        SkPath::Verb nextPathVerb = iter.peek();

                        connectOnly = nextPathVerb == SkPath::kMove_Verb ||
                                nextPathVerb == SkPath::kDone_Verb ||
                                nextPathVerb == SkPath::kClose_Verb;
                    } else {
                        connectOnly = true;
                    }
                }
                if(connectOnly) {
                    lastPoint->connectToPoint(firstPoint);
                    lastPoint = firstPoint;
                } else {
                    lastPoint = singlePathAnimator->
                            addNodeRelPos(toQPointF(pt),
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
            case SkPath::kCubic_Verb: {
                SkPoint endPt = pts[1];
                SkPoint startPt = pts[2];
                SkPoint targetPt = pts[3];
                lastPoint->setEndCtrlPtEnabled(true);
                lastPoint->moveEndCtrlPtToRelPos(
                            toQPointF(endPt));

                bool sameAsFirstPoint = pointToLen(toQPointF(targetPt) -
                                            firstPoint->getRelativePos()) < 0.1;
                bool connectOnly = false;
                if(sameAsFirstPoint) {
                    if(path.countVerbs() > verbId + 1) {
                        SkPath::Verb nextPathVerb = iter.peek();

                        connectOnly = nextPathVerb == SkPath::kMove_Verb ||
                                nextPathVerb == SkPath::kDone_Verb ||
                                nextPathVerb == SkPath::kClose_Verb;
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
                            addNodeRelPos(toQPointF(targetPt),
                                          lastPoint);
                }
                lastPoint->setStartCtrlPtEnabled(true);
                lastPoint->moveStartCtrlPtToRelPos(
                            toQPointF(startPt));
            }
                break;
            case SkPath::kClose_Verb:
                lastPoint->connectToPoint(firstPoint);
                lastPoint = firstPoint;
                singlePathAnimator->setPathClosed(true);
                break;
            case SkPath::kDone_Verb:
                goto DONE;
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
    if(singlePathAnimator) {
        addSinglePathAnimator(singlePathAnimator);
    }
}

SkPath PathAnimator::getPathAtRelFrame(const int &relFrame) {
    SkPath path;

    for(const auto& singlePath : mSinglePaths) {
        path.addPath(singlePath->getPathAtRelFrame(relFrame));
    }

    return path;
}

SkPath PathAnimator::getPathAtRelFrameF(const qreal &relFrame) {
    SkPath path;

    for(const auto& singlePath : mSinglePaths) {
        path.addPath(singlePath->getPathAtRelFrameF(relFrame));
    }

    return path;
}

void PathAnimator::selectAllPoints(Canvas * const canvas) {
    for(const auto& singlePath : mSinglePaths) {
        singlePath->selectAllPoints(canvas);
    }
}

NodePoint *PathAnimator::createNewPointOnLineNear(const QPointF &absPos,
                                                  const bool &adjust,
                                                  const qreal &canvasScaleInv) {
    for(const auto& singlePath : mSinglePaths) {
        NodePoint *pt = singlePath->createNewPointOnLineNear(absPos,
                                                             adjust,
                                                             canvasScaleInv);
        if(!pt) continue;
        return pt;
    }
    return nullptr;
}

void PathAnimator::applyTransformToPoints(const QMatrix &transform) {
    for(const auto& singlePath : mSinglePaths) {
        singlePath->applyTransformToPoints(transform);
    }
}

MovablePoint *PathAnimator::getPointAtAbsPos(
                                    const QPointF &absPtPos,
                                    const CanvasMode &currentCanvasMode,
                                    const qreal &canvasScaleInv) {
    for(const auto& singlePath : mSinglePaths) {
        MovablePoint* pt = singlePath->getPointAtAbsPos(absPtPos,
                                                     currentCanvasMode,
                                                     canvasScaleInv);
        if(!pt) continue;
        return pt;
    }

    return nullptr;
}

void PathAnimator::drawSelected(SkCanvas *canvas,
                                const CanvasMode &currentCanvasMode,
                                const SkScalar &invScale,
                                const SkMatrix &TotalTransform) {
    for(const auto& singlePath : mSinglePaths) {
        singlePath->drawSelected(canvas,
                                 currentCanvasMode,
                                 invScale,
                                 TotalTransform);
    }
}

void PathAnimator::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<stdptr<MovablePoint>> &list) {
    for(const auto& singlePath : mSinglePaths) {
        singlePath->selectAndAddContainedPointsToList(absRect, list);
    }
}

BoundingBox *PathAnimator::getParentBox() {
    return mParentBox;
}

void PathAnimator::shiftAllPointsForAllKeys(const int &by) {
    for(const auto& path : mSinglePaths) {
        path->shiftAllPointsForAllKeys(by);
    }
}

void PathAnimator::revertAllPointsForAllKeys() {
    for(const auto& path : mSinglePaths) {
        path->revertAllPointsForAllKeys();
    }
}

void PathAnimator::moveAllSinglePathsToAnimator_k(PathAnimator * const target) {
    const auto sRef = ref<PathAnimator>();
    while(!mSinglePaths.isEmpty()) {
        qsptr<VectorPathAnimator> path = mSinglePaths.at(0);
        target->addSinglePathAnimator(path);
        removeSinglePathAnimator_k(path);
        path->setParentPath(target);
    }
}

void PathAnimator::shiftAllPoints(const int &by) {
    for(const auto& path : mSinglePaths) {
        path->shiftAllPoints(by);
    }
}

void PathAnimator::revertAllPoints() {
    for(const auto& path : mSinglePaths) {
        path->revertAllPoints();
    }
}
