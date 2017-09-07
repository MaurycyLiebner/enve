#include "pathkey.h"
#include "vectorpathanimator.h"

PathKey::PathKey(VectorPathAnimator *parentAnimator) :
    Key(parentAnimator) {

}

PathKey::PathKey(const int &relFrame,
                 const SkPath &path,
                 VectorPathAnimator *parentAnimator) :
    PathKey(parentAnimator) {
    mRelFrame = relFrame;
    setElementsFromSkPath(path);
}

PathKey::PathKey(const int &relFrame,
                 const SkPath &path,
                 const QList<SkPoint> &elementsPos,
                 VectorPathAnimator *parentAnimator) :
    PathKey(parentAnimator) {
    mRelFrame = relFrame;
    mPath = path;
    mElementsPos = elementsPos;
}

NodeSettings *PathKey::getNodeSettingsForPtId(const int &ptId) {
    return ((VectorPathAnimator*)mParentAnimator)->
            getNodeSettingsForPtId(ptId);
}

const SkPoint &PathContainer::getElementPos(const int &index) const {
    return mElementsPos.at(index);
}

void PathContainer::setElementPos(const int &index,
                                  const SkPoint &pos) {
    mElementsPos.replace(index, pos);
    if(qIsNaN(pos.x()) || qIsNaN(pos.y())) {
        mPathUpdateNeeded = false;
    }
    mPathUpdateNeeded = true;
}

void PathContainer::prependElementPos(const SkPoint &pos) {
    mElementsPos.prepend(pos);
    mPathUpdateNeeded = true;
}

void PathContainer::appendElementPos(const SkPoint &pos) {
    mElementsPos.append(pos);
    if(qIsNaN(pos.x()) || qIsNaN(pos.y())) {
        mPathUpdateNeeded = false;
    }
    mPathUpdateNeeded = true;
}

void PathContainer::insertElementPos(const int &index,
                               const SkPoint &pos) {
    mElementsPos.insert(index, pos);
    if(qIsNaN(pos.x()) || qIsNaN(pos.y())) {
        mPathUpdateNeeded = false;
    }
    mPathUpdateNeeded = true;
}

void PathContainer::removeElementPosAt(const int &index) {
    mElementsPos.removeAt(index);
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
        NodeSettings *nodeSettings = getNodeSettingsForPtId(currId);

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
                break;
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

    insertElementPos(prevPtId + 2, SkPoint::Make(0., 0.));
    insertElementPos(prevPtId + 3, newPointPos);
    insertElementPos(prevPtId + 4, SkPoint::Make(0., 0.));
    if(newPtSmooth) {
        setElementPos(prevPtId + 2, newPointStart - newPointPos);
        setElementPos(prevPtId + 4, newPointEnd - newPointPos);
        setElementPos(prevPtId + 1, prevPointEnd - prevPoint);
        setElementPos(nextPtId - 1, nextPointStart - nextPoint);
    }
}
