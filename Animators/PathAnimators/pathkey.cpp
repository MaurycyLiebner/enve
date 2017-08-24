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
    mPathNeedUpdate = true;
}

void PathContainer::prependElementPos(const SkPoint &pos) {
    mElementsPos.prepend(pos);
    mPathNeedUpdate = true;
}

void PathContainer::appendElementPos(const SkPoint &pos) {
    mElementsPos.append(pos);
    mPathNeedUpdate = true;
}

void PathContainer::insertElementPos(const int &index,
                               const SkPoint &pos) {
    mElementsPos.insert(index, pos);
    mPathNeedUpdate = true;
}

void PathContainer::removeElementPosAt(const int &index) {
    mElementsPos.removeAt(index);
}

const SkPath &PathContainer::getPath() {
    if(mPathNeedUpdate) {
        updatePath();
        mPathNeedUpdate = false;
    }
    return mPath;
}

void PathContainer::updatePath() {
    mPath.reset();
    SkPoint firstPos = mElementsPos.first();
    mPath.moveTo(firstPos);
    int currId = 1;
    int elementsCount = mElementsPos.count();
    bool endEnabled = getNodeSettingsForPtId(0)->endEnabled;
    SkPoint endPos;
    if(endEnabled) {
        endPos = mElementsPos.at(1);
    } else {
        endPos = firstPos;
    }
    while(currId < elementsCount) {
        const SkPoint &targetPos = mElementsPos.at(currId);
        NodeSettings *nodeSettings = getNodeSettingsForPtId(currId);

        SkPoint startPos;
        if(nodeSettings->startEnabled) {
            startPos = mElementsPos.at(currId - 1);
        } else {
            startPos = targetPos;
        }
        mPath.cubicTo(endPos, startPos, targetPos);
        if(nodeSettings->endEnabled) {
            endPos = mElementsPos.at(currId + 1);
        } else {
            endPos = targetPos;
        }
        currId += 3;
    }
    if(mPathClosed) {
        mPath.close();
    }
}

void PathContainer::closedChanged(const bool &bT) {
    mPathClosed = bT;
    mPathNeedUpdate = true;
}

void PathContainer::setElementsFromSkPath(const SkPath &path) {
    clearElements();
    mElementsPos.reserve(path.countPoints());
    SkPath::RawIter iter = SkPath::RawIter(path);
    mPathClosed = false;
    SkPoint pts[4];
    int verbId = 0;
    SkPoint firstPoint;
    for(;;) {
        switch(iter.next(pts)) {
            case SkPath::kMove_Verb: {
                SkPoint pt = pts[0];
                firstPoint = pt;
                mElementsPos.append(pt);
            }
                break;
            case SkPath::kLine_Verb: {
                SkPoint pt = pts[1];
                bool sameAsFirstPoint = pt == firstPoint;
                bool connectOnly = false;
                if(sameAsFirstPoint) {
                    if(path.countVerbs() > verbId + 1) {
                        connectOnly = iter.peek() == SkPath::kMove_Verb;
                    } else {
                        connectOnly = true;
                    }
                }
                if(!connectOnly) {
                    mElementsPos.append(pt);
                }
            }
                break;
            case SkPath::kCubic_Verb: {
                SkPoint endPt = pts[1];
                SkPoint startPt = pts[2];
                SkPoint targetPt = pts[3];
                mElementsPos.append(endPt);


                bool sameAsFirstPoint = targetPt == firstPoint;
                bool connectOnly = false;
                if(sameAsFirstPoint) {
                    if(path.countVerbs() > verbId + 1) {
                        connectOnly = iter.peek() == SkPath::kMove_Verb;
                    } else {
                        connectOnly = true;
                    }
                }
                mElementsPos.append(startPt);
                if(!connectOnly) {
                    mElementsPos.append(targetPt);
                }
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
    mPathNeedUpdate = false;
    mPath = path;
}

void PathContainer::clearElements() {
    mElementsPos.clear();
    mPathNeedUpdate = true;
}
#include "edge.h"
void PathContainer::addNewPointAtTBetweenPts(const SkScalar &tVal,
                                             const int &id1,
                                             const int &id2,
                                             const bool &newPtSmooth) {
    int prevPtId = qMin(id1, id2);
    int nextPtId = qMax(id1, id2);
    SkPoint prevPoint = getElementPos(prevPtId);
    SkPoint prevPointEnd = getElementPos(prevPtId + 1);
    SkPoint nextPointStart = getElementPos(prevPtId + 2);
    SkPoint nextPoint = getElementPos(prevPtId + 3);
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

    insertElementPos(prevPtId + 2, newPointPos);
    insertElementPos(prevPtId + 3, newPointPos);
    insertElementPos(prevPtId + 4, newPointPos);
    nextPtId += 3;
    if(newPtSmooth) {
        setElementPos(prevPtId + 2, newPointStart);
        setElementPos(prevPtId + 4, newPointEnd);
        setElementPos(prevPtId + 1, prevPointEnd);
        setElementPos(prevPtId + 2 + 3, nextPointStart);
    }
}
