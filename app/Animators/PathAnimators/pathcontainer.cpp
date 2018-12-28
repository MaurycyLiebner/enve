#include "pathcontainer.h"
#include "nodesettings.h"
#include "skia/skqtconversions.h"
#include "pointhelpers.h"

const SkPoint &PathContainer::getElementPos(const int &index) const {
    return mElementsPos.at(index);
}

void PathContainer::setElementPos(const int &index,
                                  const SkPoint &pos) {
    mElementsPos.replace(index, pos);
    mPathUpdateNeeded = true;
}

void PathContainer::addNodeElements(int startPtIndex,
                                    const SkPoint &startPos,
                                    const SkPoint &pos,
                                    const SkPoint &endPos,
                                    const bool &saveUndoRedo) {
    if(startPtIndex == -1 || startPtIndex > mElementsPos.count()) {
        startPtIndex = mElementsPos.count();
    }
    mElementsPos.insert(startPtIndex, endPos);
    mElementsPos.insert(startPtIndex, pos);
    mElementsPos.insert(startPtIndex, startPos);
    mPathUpdateNeeded = true;
    if(saveUndoRedo) {
//        MainWindow::addUndoRedo(
//            new PathContainerAddNodeElementsUR(this,
//                                               startPtIndex,
//                                               startPos,
//                                               pos,
//                                               endPos));
    }
}

void PathContainer::removeNodeElements(const int &startPtIndex,
                                       const bool &saveUndoRedo) {
    Q_UNUSED(saveUndoRedo);

    mPathUpdateNeeded = true;
//    if(saveUndoRedo) {
//        MainWindow::addUndoRedo(
//            new PathContainerdRemoveNodeElementsUR(this,
//                                                   startPtIndex,
//                                                   mElementsPos.takeAt(startPtIndex),
//                                                   mElementsPos.takeAt(startPtIndex),
//                                                   mElementsPos.takeAt(startPtIndex)));
//    } else {
        mElementsPos.removeAt(startPtIndex);
        mElementsPos.removeAt(startPtIndex);
        mElementsPos.removeAt(startPtIndex);
//    }
}

const SkPath &PathContainer::getPath() {
    if(mPathUpdateNeeded) {
        updatePath();
        mPathUpdateNeeded = false;
    }
    return mPath;
}

void PathContainer::updatePath() {
    mPath.reset();
    if(mElementsPos.count() < 3) return;
    SkPoint firstPos = mElementsPos.at(1);
    mPath.moveTo(firstPos);
    int elementsCount = mElementsPos.count();
    bool endEnabled = getNodeSettingsForPtId(0)->endEnabled;
    SkPoint endPos;
    if(endEnabled) {
        endPos = firstPos + mElementsPos.at(2);
    } else {
        endPos = firstPos;
    }
    int currId = 4;
    while(currId < elementsCount) {
        const SkPoint &targetPos = mElementsPos.at(currId);
        NodeSettings* nodeSettings = getNodeSettingsForPtId(currId);

        SkPoint startPos;
        if(nodeSettings->startEnabled) {
            startPos = targetPos + mElementsPos.at(currId - 1);
        } else {
            startPos = targetPos;
        }
        mPath.cubicTo(endPos, startPos, targetPos);
        if(nodeSettings->endEnabled) {
            endPos = targetPos + mElementsPos.at(currId + 1);
        } else {
            endPos = targetPos;
        }
        currId += 3;
    }
    if(mPathClosed) {
        SkPoint firstStartPos;
        if(getNodeSettingsForPtId(0)->startEnabled) {
            firstStartPos = firstPos + mElementsPos.first();
        } else {
            firstStartPos = firstPos;
        }
        mPath.cubicTo(endPos, firstStartPos, firstPos);
        mPath.close();
    }
}

void PathContainer::setPathClosed(const bool &bT) {
    mPathClosed = bT;
    mPathUpdateNeeded = true;
}

QList<SkPoint> PathContainer::extractElementsFromSkPath(const SkPath &path) {
    QList<SkPoint> elements;
    elements.reserve(path.countPoints() + 2);
    SkPath::RawIter iter = SkPath::RawIter(path);
    bool pathClosed = false;
    SkPoint pts[4];
    int verbId = 0;
    SkPoint firstPoint;
    SkPoint lastTargetPt;
    for(;;) {
        switch(iter.next(pts)) {
            case SkPath::kMove_Verb: {
                SkPoint pt = pts[0];
                firstPoint = pt;
                elements.append(SkPoint::Make(0., 0.));
                elements.append(pt);
                lastTargetPt = pt;
            }
                break;
            case SkPath::kLine_Verb: {
                SkPoint pt = pts[1];
                bool sameAsFirstPoint = pt == firstPoint;
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
                if(!connectOnly) {
                    elements.append(SkPoint::Make(0., 0.));
                    elements.append(pt);
                    elements.append(SkPoint::Make(0., 0.));
                }
                lastTargetPt = pt;
            }
                break;
            case SkPath::kCubic_Verb: {
                SkPoint endPt = pts[1];
                SkPoint startPt = pts[2];
                SkPoint targetPt = pts[3];
                elements.append(endPt - lastTargetPt);

                bool sameAsFirstPoint = targetPt == firstPoint;
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
                    pathClosed = true;
                    elements.replace(0, startPt - firstPoint);
                } else {
                    elements.append(startPt - targetPt);
                    elements.append(targetPt);
                }
                lastTargetPt = targetPt;
            }
                break;
            case SkPath::kClose_Verb:
                pathClosed = true;
                break;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
            case SkPath::kDone_Verb:
                goto DONE;
        }
        verbId++;
    }
DONE:
    if(!pathClosed) {
        elements.append(SkPoint::Make(0., 0.));
    }
    return elements;
}

void PathContainer::finishedPathChange() {
    if(mPathChanged) {
        mPathChanged = false;
//        MainWindow::addUndoRedo(
//                    new PathContainerPathChangeUR(this,
//                                                        mSavedElementsPos,
//                                                        mElementsPos));
    }
}

void PathContainer::mergeNodes(const int &nodeId1, const int &nodeId2) {
    if(nodeId1 == nodeId2) return;
    int minNodeId = qMin(nodeId1, nodeId2);
    int minPtId = nodeIdToPointId(minNodeId);
    int maxNodeId = qMax(nodeId1, nodeId2);
    int maxPtId = nodeIdToPointId(maxNodeId);
    int node1PtId = nodeIdToPointId(nodeId1);
    int node2PtId = nodeIdToPointId(nodeId2);
    const SkPoint &pos1 = mElementsPos.at(node1PtId);
    const SkPoint &pos2 = mElementsPos.at(node2PtId);
    mElementsPos.replace(minPtId, (pos1 + pos2)*0.5f);

    if(qAbs(nodeId1 - nodeId2) > 1) {
        mElementsPos.replace(minPtId - 1,
                             mElementsPos.at(maxPtId - 1) +
                             mElementsPos.at(maxPtId) -
                             mElementsPos.at(minPtId));
    } else {
        mElementsPos.replace(minPtId + 1,
                             mElementsPos.at(maxPtId + 1) +
                             mElementsPos.at(maxPtId) -
                             mElementsPos.at(minPtId));
    }

    PathContainer::removeNodeAt(maxNodeId);
}

void PathContainer::setElementsFromSkPath(const SkPath &path) {
    clearElements();
    mElementsPos.reserve(path.countPoints() + 2);
    SkPath::RawIter iter = SkPath::RawIter(path);
    mPathClosed = false;
    SkPoint pts[4];
    int verbId = 0;
    SkPoint firstPoint;
    SkPoint lastTargetPt;
    for(;;) {
        switch(iter.next(pts)) {
            case SkPath::kMove_Verb: {
                SkPoint pt = pts[0];
                firstPoint = pt;
                mElementsPos.append(SkPoint::Make(0., 0.));
                mElementsPos.append(pt);
                lastTargetPt = pt;
            }
                break;
            case SkPath::kLine_Verb: {
                SkPoint pt = pts[1];
                bool sameAsFirstPoint = pt == firstPoint;
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
                if(!connectOnly) {
                    mElementsPos.append(SkPoint::Make(0., 0.));
                    mElementsPos.append(pt);
                    mElementsPos.append(SkPoint::Make(0., 0.));
                }
                lastTargetPt = pt;
            }
                break;
            case SkPath::kCubic_Verb: {
                SkPoint endPt = pts[1];
                SkPoint startPt = pts[2];
                SkPoint targetPt = pts[3];
                mElementsPos.append(endPt - lastTargetPt);

                bool sameAsFirstPoint = targetPt == firstPoint;
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
                    mPathClosed = true;
                    mElementsPos.replace(0, startPt - firstPoint);
                } else {
                    mElementsPos.append(startPt - targetPt);
                    mElementsPos.append(targetPt);
                }
                lastTargetPt = targetPt;
            }
                break;
            case SkPath::kClose_Verb:
                mPathClosed = true;
                break;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
            case SkPath::kDone_Verb:
                goto DONE;
        }
        verbId++;
    }
DONE:
    if(!mPathClosed) {
        mElementsPos.append(SkPoint::Make(0., 0.));
    }
    mPathUpdateNeeded = false;
    mPath = path;
}

void PathContainer::clearElements() {
    mElementsPos.clear();
    mPathUpdateNeeded = true;
}
#include "edge.h"
void PathContainer::addNewPointAtTBetweenPts(const SkScalar &tVal,
                                             const int &id1,
                                             const int &id2,
                                             const bool &newPtSmooth) {
    int prevPtId = qMin(id1, id2);
    int nextPtId = qMax(id1, id2);
    if(prevPtId == 1 && nextPtId != 4) {
        prevPtId = nextPtId;
        nextPtId = 1;
    }
    SkPoint prevPoint = getElementPos(prevPtId);
    SkPoint prevPointEnd = getElementPos(prevPtId + 1) + prevPoint;
    SkPoint nextPoint = getElementPos(nextPtId);
    SkPoint nextPointStart = getElementPos(nextPtId - 1) + nextPoint;
    if(nextPtId != 1) {
        nextPtId += 3;
    }
    SkPoint newPointPos;
    SkPoint newPointStart;
    SkPoint newPointEnd;
    VectorPathEdge::getNewRelPosForKnotInsertionAtTSk(
                  prevPoint,
                  &prevPointEnd,
                  &nextPointStart,
                  nextPoint,
                  &newPointPos,
                  &newPointStart,
                  &newPointEnd,
                  tVal);

    if(newPtSmooth) {
        addNodeElements(prevPtId + 2,
                        newPointStart - newPointPos,
                        newPointPos,
                        newPointEnd - newPointPos);

        startPathChange();
        setElementPos(prevPtId + 1,
                      prevPointEnd - prevPoint);
        setElementPos(nextPtId - 1,
                      nextPointStart - nextPoint);
        finishedPathChange();
    } else {
        addNodeElements(prevPtId + 2,
                        SkPoint::Make(0.f, 0.f),
                        newPointPos,
                        SkPoint::Make(0.f, 0.f));
    }
}

void PathContainer::setCtrlsModeForNode(const int &nodeId,
                                        const CtrlsMode &mode) {
    int nodePtId = nodeIdToPointId(nodeId);
    QPointF startPos = SkPointToQPointF(mElementsPos.at(nodePtId - 1));
    QPointF pos = SkPointToQPointF(mElementsPos.at(nodePtId));
    QPointF endPos = SkPointToQPointF(mElementsPos.at(nodePtId + 1));
    QPointF newStartPos;
    QPointF newEndPos;
    if(mode == CtrlsMode::CTRLS_SYMMETRIC) {
        getCtrlsSymmetricPos(endPos,
                             startPos,
                             pos,
                             &newEndPos,
                             &newStartPos);
        mElementsPos.replace(nodePtId - 1, QPointFToSkPoint(newStartPos));
        mElementsPos.replace(nodePtId + 1, QPointFToSkPoint(newEndPos));
    } else if(mode == CtrlsMode::CTRLS_SMOOTH) {
        getCtrlsSmoothPos(endPos,
                          startPos,
                          pos,
                          &newEndPos,
                          &newStartPos);
        mElementsPos.replace(nodePtId - 1, QPointFToSkPoint(newStartPos));
    }
}

void PathContainer::applyTransformToPoints(const QMatrix &transform) {
    for(int i = 1; i < mElementsPos.count(); i += 3) {
        QPointF nodePos = SkPointToQPointF(mElementsPos.at(i));
        QPointF newNodePos = transform.map(nodePos);
        setElementPos(i, QPointFToSkPoint(
                          newNodePos) );
        QPointF startPos = nodePos + SkPointToQPointF(mElementsPos.at(i - 1));
        setElementPos(i - 1, QPointFToSkPoint(
                          transform.map(startPos) - newNodePos) );
        QPointF endPos = nodePos + SkPointToQPointF(mElementsPos.at(i + 1));
        setElementPos(i + 1, QPointFToSkPoint(
                          transform.map(endPos) - newNodePos));
    }
    mPathUpdateNeeded = true;
}
