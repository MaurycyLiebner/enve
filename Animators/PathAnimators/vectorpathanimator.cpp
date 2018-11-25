#include "vectorpathanimator.h"
#include "Animators/pathanimator.h"
#include "Boxes/boundingbox.h"
#include "edge.h"
#include "nodepoint.h"
#include "canvas.h"
#include "undoredo.h"

VectorPathAnimator::VectorPathAnimator(PathAnimator *pathAnimator) :
    Animator() {
    prp_setName("path");
    mParentPathAnimator = pathAnimator;
}

VectorPathAnimator::VectorPathAnimator(const QList<NodeSettings> &settingsList,
                                       PathAnimator *pathAnimator) :
    VectorPathAnimator(pathAnimator) {
    foreach(const NodeSettings &nodeSetting, settingsList) {
        mNodeSettings << SPtrCreate(NodeSettings)(nodeSetting);
    }
}

VectorPathAnimator::VectorPathAnimator(const QList<NodeSettings> &settingsList,
                                       const QList<SkPoint> &posList,
                                       PathAnimator *pathAnimator) :
    VectorPathAnimator(settingsList, pathAnimator) {
    mElementsPos = posList;
    updatePath();
    updateNodePointsFromElements();
}

void VectorPathAnimator::prp_setAbsFrame(const int &frame) {
    Animator::prp_setAbsFrame(frame);
    //setCurrentPath(getPathAtRelFrame(frame));
    if(prp_hasKeys()) {
        setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame, false));
    }
}

void VectorPathAnimator::setElementsFromSkPath(const SkPath &path) {
    PathContainer::setElementsFromSkPath(path);
    updateNodePointsFromElements();
}

void VectorPathAnimator::anim_saveCurrentValueToKey(PathKey* key) {
    key->setElementsFromSkPath(getPath());
}

void VectorPathAnimator::setCtrlsModeForNode(const int &nodeId,
                                             const CtrlsMode &mode) {
    NodeSettings newSettings = *mNodeSettings.at(nodeId);
    newSettings.ctrlsMode = mode;
    newSettings.endEnabled = true;
    newSettings.startEnabled = true;
    replaceNodeSettingsForNodeId(nodeId, newSettings);
    int nodePtId = nodeIdToPointId(nodeId);
    QPointF pos = SkPointToQPointF(mElementsPos.at(nodePtId));
    QPointF startPos = SkPointToQPointF(mElementsPos.at(nodePtId - 1)) + pos;
    QPointF endPos = SkPointToQPointF(mElementsPos.at(nodePtId + 1)) + pos;
    QPointF newStartPos;
    QPointF newEndPos;
    if(mode == CtrlsMode::CTRLS_SYMMETRIC) {
        getCtrlsSymmetricPos(endPos,
                             startPos,
                             pos,
                             &newEndPos,
                             &newStartPos);
        mElementsPos.replace(nodePtId - 1, QPointFToSkPoint(newStartPos - pos));
        mElementsPos.replace(nodePtId + 1, QPointFToSkPoint(newEndPos));
    } else if(mode == CtrlsMode::CTRLS_SMOOTH) {
        getCtrlsSmoothPos(endPos,
                          startPos,
                          pos,
                          &newEndPos,
                          &newStartPos);
        mElementsPos.replace(nodePtId - 1, QPointFToSkPoint(newStartPos - pos));
    }

    //mParentPath->schedulePathUpdate();
}

void VectorPathAnimator::startPathChange() {
    if(mPathChanged) return;
    if(prp_isRecording()) {
        if(prp_isKeyOnCurrentFrame()) return;
        anim_saveCurrentValueAsKey();
    }
    if(prp_isKeyOnCurrentFrame()) {
        SPtrGetAs(anim_mKeyOnCurrentFrame, PathKey)->startPathChange();
        anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
    } else {
        PathContainer::startPathChange();
        prp_updateInfluenceRangeAfterChanged();
    }
    mPathChanged = true;
}

void VectorPathAnimator::cancelPathChange() {
    if(!mPathChanged) return;
    if(prp_isKeyOnCurrentFrame()) {
        SPtrGetAs(anim_mKeyOnCurrentFrame, PathKey)->cancelPathChange();
        anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
    } else {
        PathContainer::cancelPathChange();
        prp_updateInfluenceRangeAfterChanged();
    }
    mPathChanged = false;
}

void VectorPathAnimator::finishedPathChange() {
    if(!mPathChanged) return;
    if(prp_isKeyOnCurrentFrame()) {
        SPtrGetAs(anim_mKeyOnCurrentFrame, PathKey)->finishedPathChange();
        anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
    } else {
        PathContainer::finishedPathChange();
        prp_updateInfluenceRangeAfterChanged();
    }
    mPathChanged = false;
}

void VectorPathAnimator::setElementPos(const int &index,
                                       const SkPoint &pos) {
    if(prp_isKeyOnCurrentFrame()) {
        SPtrGetAs(anim_mKeyOnCurrentFrame, PathKey)->setElementPos(index, pos);
        anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
    } else {
        PathContainer::setElementPos(index, pos);
        prp_updateInfluenceRangeAfterChanged();
    }
}

void VectorPathAnimator::anim_addKeyAtRelFrame(const int& relFrame) {
    if(!anim_mIsRecording) prp_setRecording(true);

    PathKeySPtr newKey = anim_getKeyAtRelFrame(relFrame)->ref<PathKey>();

    if(newKey == nullptr) {
        newKey = SPtrCreate(PathKey)(relFrame, getPathAtRelFrame(relFrame),
                                          this, mPathClosed);
        anim_appendKey(newKey, true, false);
    } else {
        newKey->setElementsFromSkPath(getPathAtRelFrame(relFrame));
    }
}

void VectorPathAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) prp_setRecording(true);

    if(anim_mKeyOnCurrentFrame == nullptr) {
        auto newKey = SPtrCreate(PathKey)(anim_mCurrentRelFrame,
                                          getPath(),
                                          mElementsPos,
                                          ref<VectorPathAnimator>(),
                                          mPathClosed);
        anim_appendKey(newKey, true, false);
        anim_mKeyOnCurrentFrame = newKey.get();
    } else {
        anim_saveCurrentValueToKey(SPtrGetAs(anim_mKeyOnCurrentFrame, PathKey));
    }
}

SkPath VectorPathAnimator::getPathAtRelFrame(const int &relFrame,
                                             const bool &considerCurrent,
                                             const bool &interpolate) {
    Q_UNUSED(considerCurrent);
    Q_UNUSED(interpolate);
    if(mElementsUpdateNeeded) {
        finalizeNodesRemove();
    }
    //if(relFrame == anim_mCurrentRelFrame && considerCurrent) return getPath();
    SkPath pathToRuturn;
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(&prevId, &nextId, relFrame) ) {
        if(prevId == nextId) {
            pathToRuturn = SPtrGetAs(anim_mKeys.at(prevId), PathKey)->getPath();
        } else {
            PathKeySPtr prevKey = anim_mKeys.at(prevId)->ref<PathKey>();
            PathKeySPtr nextKey = anim_mKeys.at(nextId)->ref<PathKey>();
            int prevRelFrame = prevKey->getRelFrame();
            int nextRelFrame = nextKey->getRelFrame();
            SkScalar weight = (static_cast<SkScalar>(relFrame) - prevRelFrame)/
                    (nextRelFrame - prevRelFrame);
            nextKey->getPath().interpolate(prevKey->getPath(),
                                           weight, &pathToRuturn);
        }
    } else {
        pathToRuturn = getPath();
    }

    if(mElementsUpdateNeeded) {
        if(relFrame == anim_mCurrentRelFrame) {
            mElementsUpdateNeeded = false;
            finalizeNodesRemove();
            setElementsFromSkPath(pathToRuturn);
        }
    }

    return pathToRuturn;
}

SkPath VectorPathAnimator::getPathAtRelFrameF(const qreal &relFrame,
                                              const bool &considerCurrent,
                                              const bool &interpolate) {
    Q_UNUSED(considerCurrent);
    Q_UNUSED(interpolate);
    if(mElementsUpdateNeeded) {
        finalizeNodesRemove();
    }
    //if(relFrame == anim_mCurrentRelFrame && considerCurrent) return getPath();
    SkPath pathToRuturn;
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrameF(&prevId, &nextId, relFrame) ) {
        if(prevId == nextId) {
            pathToRuturn = SPtrGetAs(anim_mKeys.at(prevId), PathKey)->getPath();
        } else {
            PathKeySPtr prevKey = anim_mKeys.at(prevId)->ref<PathKey>();
            PathKeySPtr nextKey = anim_mKeys.at(nextId)->ref<PathKey>();
            int prevRelFrame = prevKey->getRelFrame();
            int nextRelFrame = nextKey->getRelFrame();
            SkScalar weight = (static_cast<SkScalar>(relFrame) - prevRelFrame)/
                    (nextRelFrame - prevRelFrame);
            nextKey->getPath().interpolate(prevKey->getPath(),
                                           weight, &pathToRuturn);
        }
    } else {
        pathToRuturn = getPath();
    }

    if(mElementsUpdateNeeded) {
        if(isZero(relFrame - anim_mCurrentRelFrame)) {
            mElementsUpdateNeeded = false;
            finalizeNodesRemove();
            setElementsFromSkPath(pathToRuturn);
        }
    }

    return pathToRuturn;
}

void VectorPathAnimator::replaceNodeSettingsForNodeId(const int &nodeId,
                                                      const NodeSettings &settings,
                                                      const bool &saveUndoRedo) {
    if(saveUndoRedo) {
//        addUndoRedo(new VectorPathAnimatorReplaceNodeSettingsUR(
//                        this, nodeId,
//                        *mNodeSettings.at(nodeId),
//                        settings));
    }
    *mNodeSettings.at(nodeId) = settings;
}

NodeSettings* VectorPathAnimator::insertNodeSettingsForNodeId(
        const int &nodeId,
        const NodeSettings &settings,
        const bool &saveUndoRedo) {
    NodeSettingsSPtr newSettings = SPtrCreate(NodeSettings)();
    *newSettings = settings;
    mNodeSettings.insert(nodeId, newSettings);
    if(saveUndoRedo) {
//        addUndoRedo(new VectorPathAnimatorInsertNodeSettingsUR(this,
//                                                               nodeId,
//                                                               settings));
    }
    return newSettings.get();
}

void VectorPathAnimator::anim_removeKey(const KeySPtr& keyToRemove,
                                        const bool &saveUndoRedo) {
    Animator::anim_removeKey(keyToRemove, saveUndoRedo);
    mElementsUpdateNeeded = true;
    //    if(anim_mKeys.count() == 0) {
    //        setElementsFromSkPath(getPath());
    //    }
}

void VectorPathAnimator::anim_moveKeyToRelFrame(Key *key,
                                                const int &newFrame,
                                                const bool &saveUndoRedo,
                                                const bool &finish) {
    Animator::anim_moveKeyToRelFrame(key, newFrame,
                                     saveUndoRedo, finish);
    mElementsUpdateNeeded = true;
}

void VectorPathAnimator::anim_appendKey(const KeySPtr& newKey,
                                        const bool &saveUndoRedo,
                                        const bool &update) {
    Animator::anim_appendKey(newKey, saveUndoRedo, update);
    mElementsUpdateNeeded = true;
}

VectorPathEdge *VectorPathAnimator::getEdge(const QPointF &absPos,
                                            const qreal &canvasScaleInv) {
    qreal pressedT;
    NodePoint *prevPoint = nullptr;
    NodePoint *nextPoint = nullptr;
    if(getTAndPointsForMouseEdgeInteraction(absPos, pressedT,
                                            &prevPoint, &nextPoint,
                                            canvasScaleInv)) {
        if(pressedT > 0.0001 && pressedT < 0.9999 && prevPoint && nextPoint) {
            VectorPathEdge *edge = prevPoint->getNextEdge();
            edge->setPressedT(pressedT);
            return edge;
        } else {
            return nullptr;
        }
    }
    return nullptr;
}

bool VectorPathAnimator::getTAndPointsForMouseEdgeInteraction(
                                          const QPointF &absPos,
                                          qreal& pressedT,
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

    foreach(const NodePointSPtr& nodePoint, mPoints) {
        VectorPathEdge *edgeT = nodePoint->getNextEdge();
        if(edgeT == nullptr) continue;
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
            *prevPoint = nodePoint.get();
        }
    }
    QPointF distT = nearestPos - relPos;
    if(qAbs(distT.x()) > maxDistX ||
       qAbs(distT.y()) > maxDistY) return false;

    pressedT = nearestT;
    if(*prevPoint == nullptr) return false;

    *nextPoint = (*prevPoint)->getNextPoint();
    if(*nextPoint == nullptr) return false;

    return true;
}

NodePoint* VectorPathAnimator::createNodePointAndAppendToList() {
    NodePointSPtr nodePt = SPtrCreate(NodePoint)(this);
    appendToPointsList(nodePt);
    return nodePt.get();
}

NodePoint *VectorPathAnimator::createNewPointOnLineNear(
                                    const QPointF &absPos,
                                    const bool &adjust,
                                    const qreal &canvasScaleInv) {
    qreal pressedT;
    NodePoint *prevPoint = nullptr;
    NodePoint *nextPoint = nullptr;
    if(getTAndPointsForMouseEdgeInteraction(absPos, pressedT,
                                            &prevPoint, &nextPoint,
                                            canvasScaleInv)) {
        if(pressedT > 0.0001 && pressedT < 0.9999) {
            int prevNodeId = prevPoint->getNodeId();
            int newNodeId = prevNodeId + 1;

            NodeSettings *prevNodeSettings =
                    getNodeSettingsForNodeId(prevNodeId);
            NodeSettings *nextNodeSettings;
            int nextNodeId;
            int prevPtId = nodeIdToPointId(prevNodeId);
            int nextPtId;
            if(nextPoint->getNodeId() == 0) {
                nextNodeSettings =
                        getNodeSettingsForNodeId(0);

                nextNodeId = 0;
                nextPtId = 1;
            } else {
                nextNodeSettings = getNodeSettingsForNodeId(prevNodeId + 1);
                nextPtId = nodeIdToPointId(prevNodeId + 1);
                nextNodeId = prevNodeId + 2;
            }

            bool newPtSmooth = false;

            
            NodePoint* newPoint = createNodePointAndAppendToList();

            NodeSettings *newNodeSettings;
            if(adjust) {
                if(!prevNodeSettings->endEnabled &&
                   !nextNodeSettings->startEnabled) {
                    newNodeSettings = insertNodeSettingsForNodeId(newNodeId,
                                                NodeSettings(false, false,
                                                             CTRLS_CORNER));
                } else {
                    newPtSmooth = true;
                    newNodeSettings = insertNodeSettingsForNodeId(newNodeId,
                                                NodeSettings(true, true,
                                                             CTRLS_SMOOTH));
                    if(prevNodeSettings->ctrlsMode == CTRLS_SYMMETRIC &&
                       prevNodeSettings->endEnabled &&
                       prevNodeSettings->startEnabled) {
                        replaceNodeSettingsForNodeId(
                                    prevNodeId,
                                    NodeSettings(true, true,
                                                 CTRLS_SMOOTH));
                    }
                    if(nextNodeSettings->ctrlsMode == CTRLS_SYMMETRIC &&
                       nextNodeSettings->endEnabled &&
                       nextNodeSettings->startEnabled) {
                        replaceNodeSettingsForNodeId(
                                    nextNodeId,
                                    NodeSettings(true, true,
                                                 CTRLS_SMOOTH));
                    }
                }

            } else {
                newNodeSettings = insertNodeSettingsForNodeId(newNodeId,
                                            NodeSettings(false, false,
                                                         CTRLS_CORNER));
            }
            newPoint->setCurrentNodeSettings(newNodeSettings);

            nextPoint->setPointAsPrevious(newPoint);
            prevPoint->setPointAsNext(newPoint);
            updateNodePointIds();

            if(anim_mKeys.isEmpty()) {
                addNewPointAtTBetweenPts(static_cast<SkScalar>(pressedT),
                                         prevPtId,
                                         nextPtId,
                                         newPtSmooth);
                nextPtId = nodeIdToPointId(nextNodeId);
                prevPoint->setElementsPos(SkPointToQPointF(mElementsPos.at(prevPtId - 1)),
                                         SkPointToQPointF(mElementsPos.at(prevPtId)),
                                         SkPointToQPointF(mElementsPos.at(prevPtId + 1)));
                newPoint->setElementsPos(SkPointToQPointF(mElementsPos.at(prevPtId + 2)),
                                         SkPointToQPointF(mElementsPos.at(prevPtId + 3)),
                                         SkPointToQPointF(mElementsPos.at(prevPtId + 4)));
                nextPoint->setElementsPos(SkPointToQPointF(mElementsPos.at(nextPtId - 1)),
                                         SkPointToQPointF(mElementsPos.at(nextPtId)),
                                         SkPointToQPointF(mElementsPos.at(nextPtId + 1)));
            } else {
                foreach(const KeySPtr &key, anim_mKeys) {
                    SPtrGetAs(key, PathKey)->
                            addNewPointAtTBetweenPts(static_cast<SkScalar>(pressedT),
                                                     prevPtId,
                                                     nextPtId,
                                                     newPtSmooth);
                }
            }
            setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame));
            return newPoint;
        }
    }
    return nullptr;
}

void VectorPathAnimator::updateNodePointsFromElements() {
    if(mElementsUpdateNeeded) {
        mElementsUpdateNeeded = false;
        finalizeNodesRemove();
        setElementsFromSkPath(getPath());
    }
    if(mFirstPoint != nullptr) {
        mFirstPoint->setPointAsPrevious(nullptr); // make sure path not closed
    }
    NodePoint *currOldNode = mFirstPoint;
    NodePoint* newFirstPt = nullptr;
    int elementsCount = mElementsPos.count();
    int nodesCount = elementsCount/3;
    if(nodesCount == 0) return;
    NodePoint *lastP = nullptr;
    for(int i = 0; i < nodesCount; i++) {
        int nodePtId = nodeIdToPointId(i);
        NodePoint* newP;
        if(currOldNode == nullptr) {
            newP = createNodePointAndAppendToList();
        } else {
            newP = currOldNode;
            currOldNode = currOldNode->getNextPoint();
        }
        newP->setCurrentNodeSettings(getNodeSettingsForNodeId(i));
        newP->setElementsPos(SkPointToQPointF(mElementsPos.at(nodePtId - 1)),
                             SkPointToQPointF(mElementsPos.at(nodePtId)),
                             SkPointToQPointF(mElementsPos.at(nodePtId + 1)));
        newP->setNodeId(i);

        if(i == 0) {
            newFirstPt = newP;
        } else {
            newP->setPointAsPrevious(lastP);
        }
        lastP = newP;
    }

    while(currOldNode != nullptr) {
        NodePoint *oldOldNode = currOldNode;
        currOldNode = currOldNode->getNextPoint();
        if(oldOldNode->isSelected()) {
            if(mParentPathAnimator != nullptr) {
                BoundingBox *parentBox = mParentPathAnimator->getParentBox();
                if(parentBox != nullptr) {
                    Canvas *parentCanvas = parentBox->getParentCanvas();
                    if(parentCanvas != nullptr) {
                        parentCanvas->removePointFromSelection(oldOldNode);
                    }
                }
            }
        }
        mPoints.removeOne(oldOldNode->ref<NodePoint>());
    }
    if(mPathClosed) {
        newFirstPt->setPointAsPrevious(lastP);
    } else {
        lastP->setPointAsNext(nullptr);
    }
    setFirstPoint(newFirstPt);
}

PathAnimator *VectorPathAnimator::getParentPathAnimator() {
    return mParentPathAnimator;
}

void VectorPathAnimator::finalizeNodesRemove() {
    qSort(mNodesToRemove.begin(), mNodesToRemove.end());
    for(int i = mNodesToRemove.count() - 1; i >= 0; i--) {
        const int &nodeId = mNodesToRemove.at(i);
        removeNodeSettingsAt(nodeId);
        PathContainer::removeNodeAt(nodeId);
        foreach(const KeySPtr &key, anim_mKeys) {
            SPtrGetAs(key, PathKey)->removeNodeAt(nodeId);
        }
    }
    mNodesToRemove.clear();
}

void VectorPathAnimator::removeNodeAtAndApproximate(const int &nodeId) {
    if(nodeId <= 0 || nodeId >= mElementsPos.count()/3) return;
    removeNodeSettingsAt(nodeId, true);

    setNodeCtrlsMode(nodeId + 1, CtrlsMode::CTRLS_CORNER);
    setNodeCtrlsMode(nodeId - 1, CtrlsMode::CTRLS_CORNER);
    PathContainer::removeNodeAtAndApproximate(nodeId);
    foreach(const KeySPtr &key, anim_mKeys) {
        SPtrGetAs(key, PathKey)->removeNodeAtAndApproximate(nodeId);
    }
    mElementsUpdateNeeded = true;
}

void VectorPathAnimator::removeNodeSettingsAt(const int &id,
                                              const bool &saveUndoRedo) {
    if(saveUndoRedo) {
        NodeSettings nodeSettings = *mNodeSettings.takeAt(id);
//        addUndoRedo(new VectorPathAnimatorRemoveNodeSettingsUR(this,
//                                                               id,
//                                                               nodeSettings));
    } else {
        mNodeSettings.removeAt(id);
    }
}

void VectorPathAnimator::removeNodeAt(const int &nodeId,
                                      const bool &saveUndoRedo) {
    Q_UNUSED(saveUndoRedo)
    if(nodeId < 0 || nodeId >= mElementsPos.count()/3) return;
    mNodesToRemove.append(nodeId);
    mElementsUpdateNeeded = true;
    prp_updateInfluenceRangeAfterChanged();
}

NodePoint *VectorPathAnimator::addNodeAbsPos(
        const QPointF &absPos,
        NodePoint *targetPt) {
    BoundingBox *parentBox = mParentPathAnimator->getParentBox();
    return addNodeRelPos(parentBox->mapAbsPosToRel(absPos),
                          targetPt);
}

NodePoint *VectorPathAnimator::addNodeRelPos(
        const QPointF &relPos,
        NodePoint *targetPt) {
    return addNodeRelPos(relPos, relPos, relPos, targetPt);
}

void VectorPathAnimator::selectAllPoints(Canvas *canvas) {
    Q_FOREACH(const NodePointSPtr& point, mPoints) {
        canvas->addPointToSelection(point.get());
    }
}

void VectorPathAnimator::applyTransformToPoints(const QMatrix &transform) {
    PathContainer::applyTransformToPoints(transform);
    foreach(const KeySPtr &key, anim_mKeys) {
        SPtrGetAs(key, PathKey)->applyTransformToPoints(transform);
    }
    updateNodePointsFromElements();
}

MovablePoint *VectorPathAnimator::getPointAtAbsPos(
                        const QPointF &absPtPos,
                        const CanvasMode &currentCanvasMode,
                        const qreal &canvasScaleInv) {
    Q_FOREACH(const NodePointSPtr& point, mPoints) {
        MovablePoint* pointToReturn =
                point->getPointAtAbsPos(absPtPos,
                                        currentCanvasMode,
                                        canvasScaleInv);
        if(pointToReturn == nullptr) continue;
        return pointToReturn;
    }
    return nullptr;
}

void VectorPathAnimator::setParentPath(PathAnimator *parentPath) {
    if(mParentPathAnimator != nullptr && parentPath != nullptr) {
        if(mParentPathAnimator->getParentBox() != parentPath->getParentBox()) {
            Q_FOREACH(const NodePointSPtr& point, mPoints) {
                point->setParentTransformAnimator(
                            parentPath->getParentBox()->
                                getTransformAnimator());
            }
        }
    }
    mParentPathAnimator = parentPath;
}

void VectorPathAnimator::selectAndAddContainedPointsToList(
        const QRectF &absRect,
        QList<MovablePoint *> *list) {
    Q_FOREACH(const NodePointSPtr& point, mPoints) {
        point->rectPointsSelection(absRect, list);
    }
}

void VectorPathAnimator::startAllPointsTransform() {
    Q_FOREACH(const NodePointSPtr& point, mPoints) {
        point->startTransform();
    }
}

void VectorPathAnimator::finishAllPointsTransform() {
    Q_FOREACH(const NodePointSPtr& point, mPoints) {
        point->finishTransform();
    }
}

void VectorPathAnimator::drawSelected(SkCanvas *canvas,
                                      const CanvasMode &currentCanvasMode,
                                      const SkScalar &invScale,
                                      const SkMatrix &combinedTransform) {
    Q_UNUSED(combinedTransform);

    if(currentCanvasMode == CanvasMode::MOVE_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            NodePoint *point = getNodePtWithNodeId(i);
            point->drawSk(canvas, currentCanvasMode, invScale,
                          prp_isKeyOnCurrentFrame());
        }
    } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            NodePoint *point = getNodePtWithNodeId(i);
            if(point->isEndPoint() || point->isSelected()) {
                point->drawSk(canvas, currentCanvasMode, invScale,
                              prp_isKeyOnCurrentFrame());
            }
        }
    }
}

void VectorPathAnimator::connectPoints(NodePoint *pt1,
                                       NodePoint* pt2) {
    pt1->connectToPoint(pt2);
    setPathClosed(true);
}

void VectorPathAnimator::appendToPointsList(const NodePointSPtr& pt) {
    mPoints << pt;
}

void VectorPathAnimator::setFirstPoint(NodePoint* pt) {
    if(mFirstPoint != nullptr) {
        mFirstPoint->setSeparateNodePoint(false);
    }
    mFirstPoint = pt;
    if(mFirstPoint != nullptr) {
        mFirstPoint->setSeparateNodePoint(true);
    }
}

void VectorPathAnimator::moveElementPosSubset(int firstId,
                                              int count,
                                              int targetId) {
    PathContainer::moveElementPosSubset(firstId, count, targetId);
    foreach(const KeySPtr &key, anim_mKeys) {
        SPtrGetAs(key, PathKey)->moveElementPosSubset(firstId, count, targetId);
    }
}

void VectorPathAnimator::revertNodeSettingsSubset(
            const int &firstId,
            int count) {
    if(count == -1) {
        count = mNodeSettings.count() - firstId;
    }
    int lastId = firstId + count - 1;
    for(int i = 0; i < count; i++) {
        mNodeSettings.move(lastId, firstId + i);
    }
}

VectorPathAnimator * VectorPathAnimator::connectWith(VectorPathAnimator *srcPath) {
    QList<int> keyFrames;
    QList<QList<SkPoint> > newKeysData;
    QList<NodeSettings> newNodeSettings;
    foreach(const NodeSettingsSPtr& setting, mNodeSettings) {
        newNodeSettings << *setting;
    }
    QList<NodeSettingsSPtr> srcNodeSettings;
    srcPath->getNodeSettingsList(srcNodeSettings);
    foreach(const NodeSettingsSPtr& setting, srcNodeSettings) {
        newNodeSettings << *setting;
    }

    getKeysDataForConnection(this, srcPath, &keyFrames, &newKeysData, false);
    getKeysDataForConnection(srcPath, this, &keyFrames, &newKeysData, true);

    QList<SkPoint> srcElements;
    srcPath->getElementPosList(&srcElements);
    QList<SkPoint> defPosList;
    defPosList.append(mElementsPos);
    defPosList.append(srcElements);

    VectorPathAnimatorQSPtr newAnimator =
            SPtrCreate(VectorPathAnimator)(newNodeSettings,
                                           defPosList, mParentPathAnimator);
    int idT = 0;
    foreach(const QList<SkPoint> &posList, newKeysData) {
        int relFrame = keyFrames.at(idT);
        PathKeySPtr newKey = SPtrCreate(PathKey)(relFrame, posList,
                                                 newAnimator, false);
        newAnimator->anim_appendKey(newKey);
        idT++;
    }

    mParentPathAnimator->addSinglePathAnimator(newAnimator);

    srcPath->removeFromParent();
    removeFromParent();
    return newAnimator.get();
}

void VectorPathAnimator::revertElementPosSubset(
        const int &firstId,
        int count) {
    PathContainer::revertElementPosSubset(firstId, count);
    foreach(const KeySPtr &key, anim_mKeys) {
        SPtrGetAs(key.get(), PathKey)->revertElementPosSubset(firstId, count);
    }
}

void VectorPathAnimator::disconnectPoints(NodePoint *pt1,
                                          NodePoint *pt2) {
    NodePoint *prevPt;
    NodePoint *nextPt;
    if(pt1->getNextPoint() == pt2) {
        prevPt = pt1;
        nextPt = pt2;
    } else {
        prevPt = pt2;
        nextPt = pt1;
    }

    if(mPathClosed) {
        int oldPrevPtId = prevPt->getPtId();
        moveElementPosSubset(0, oldPrevPtId + 2, -1);
        setPathClosed(false);
        updateNodePointsFromElements();
    } else {
        VectorPathAnimatorQSPtr newSinglePath;
        QList<NodeSettings> nodeSettings;
        {
            int firstNodeForNew = nextPt->getNodeId();
            int countNds = mNodeSettings.count() - firstNodeForNew;

            for(int i = 0; i < countNds; i++) {
                nodeSettings.append(*mNodeSettings.at(firstNodeForNew));
                removeNodeSettingsAt(firstNodeForNew, true);
            }
        }
        if(prp_hasKeys()) {
            newSinglePath = SPtrCreate(VectorPathAnimator)(
                        nodeSettings, mParentPathAnimator);
            foreach(const KeySPtr &key, anim_mKeys) {
                SPtrGetAs(key, PathKey)->createNewKeyFromSubsetForPath(
                                             newSinglePath,
                                             nextPt->getPtId() - 1,
                                             -1);
            }
            mElementsUpdateNeeded = true;
        } else {
            QList<SkPoint> elementsPos =
                    takeElementsPosSubset(nextPt->getPtId() - 1, -1);


            newSinglePath = SPtrCreate(VectorPathAnimator)(
                        nodeSettings, elementsPos, mParentPathAnimator);
            mPathUpdateNeeded = true;
            updateNodePointsFromElements();
        }

        mParentPathAnimator->addSinglePathAnimator(newSinglePath);

        prp_updateInfluenceRangeAfterChanged();
    }
}

NodePoint* VectorPathAnimator::createNewNode(const int &targetNodeId,
                                             const QPointF &startRelPos,
                                             const QPointF &relPos,
                                             const QPointF &endRelPos,
                                             const NodeSettings &nodeSettings) {
    NodeSettings* nodeSettingsPtr =
            insertNodeSettingsForNodeId(targetNodeId,
                                        nodeSettings);
    int nodePtId = nodeIdToPointId(targetNodeId) - 1;
    addNodeElements(nodePtId,
                    QPointFToSkPoint(startRelPos - relPos),
                    QPointFToSkPoint(relPos),
                    QPointFToSkPoint(endRelPos - relPos));
    foreach(const KeySPtr &key, anim_mKeys) {
        SPtrGetAs(key, PathKey)->addNodeElements(nodePtId,
                        QPointFToSkPoint(startRelPos - relPos),
                        QPointFToSkPoint(relPos),
                        QPointFToSkPoint(endRelPos - relPos));
    }

    NodePoint* newP = createNodePointAndAppendToList();

    newP->setCurrentNodeSettings(nodeSettingsPtr);
    newP->setElementsPos(startRelPos - relPos,
                         relPos,
                         endRelPos - relPos);
    return newP;
}

NodePoint *VectorPathAnimator::addNodeRelPos(
        const QPointF &startRelPos,
        const QPointF &relPos,
        const QPointF &endRelPos,
        NodePoint *targetPt,
        const NodeSettings &nodeSettings,
        const bool &saveUndoRedo) {
    Q_UNUSED(saveUndoRedo);
    int targetNodeId;
    bool changeFirstPt = false;
    if(targetPt == nullptr) {
        targetNodeId = 0;
        changeFirstPt = true;
    } else {
        if(targetPt->getNodeId() == 0 &&
           mFirstPoint->hasNextPoint()) {
            targetNodeId = 0;
            changeFirstPt = true;
        } else {
            targetNodeId = targetPt->getNodeId() + 1;
        }
    }

    NodePoint* newP = createNewNode(targetNodeId, startRelPos,
                                     relPos, endRelPos, nodeSettings);

    if(changeFirstPt) {
        setFirstPoint(newP);
    }
    if(targetPt != nullptr) {
        targetPt->connectToPoint(newP);
    }
    updateNodePointIds();
    prp_updateInfluenceRangeAfterChanged();

//    if(saveUndoRedo) {
//        int targetPtId = -1;
//        if(targetPt != nullptr) targetPtId = targetPt->getPtId();
//        addUndoRedo(new AddPointToVectorPathAnimatorUndoRedo(this,
//                                                             startRelPos,
//                                                             relPos,
//                                                             endRelPos,
//                                                             targetPtId,
//                                                             nodeSettings,
//                                                             newP->getPtId()));
//    }

    return newP;
}

NodePoint *VectorPathAnimator::addNodeRelPos(
        const QPointF &startRelPos,
        const QPointF &relPos,
        const QPointF &endRelPos,
        const int &targetPtId,
        const NodeSettings &nodeSettings,
        const bool &saveUndoRedo) {
    NodePoint* ptT = nullptr;
    if(!mPoints.isEmpty()) {
        if(targetPtId == -1) {
            ptT = mPoints.last().get();
        } else {
            ptT = mPoints.at(targetPtId).get();
        }
    }
    return addNodeRelPos(startRelPos, relPos, endRelPos,
                         ptT, nodeSettings, saveUndoRedo);
}

void VectorPathAnimator::updateNodePointIds() {
    if(mFirstPoint == nullptr) return;
    int pointId = 0;
    NodePoint* nextPoint = mFirstPoint;
    while(true) {
        nextPoint->setNodeId(pointId);
        pointId++;
        nextPoint = nextPoint->getNextPoint();
        if(nextPoint == nullptr || nextPoint == mFirstPoint) break;
    }
}

void VectorPathAnimator::shiftAllPointsForAllKeys(const int &by) {
    PathContainer::shiftAllPoints(by);
    foreach(const KeySPtr &key, anim_mKeys) {
        SPtrGetAs(key, PathKey)->shiftAllPoints(by);
    }
    setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame, false));
}

void VectorPathAnimator::revertAllPointsForAllKeys() {
    PathContainer::revertAllPoints();
    revertAllNodeSettings();
    foreach(const KeySPtr &key, anim_mKeys) {
        SPtrGetAs(key, PathKey)->revertAllPoints();
    }
    setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame, false));
}

void VectorPathAnimator::removeFromParent() {
    if(mParentPathAnimator == nullptr) return;
    mParentPathAnimator->removeSinglePathAnimator(ref<VectorPathAnimator>());
}

void VectorPathAnimator::mergeNodes(const int &nodeId1,
                                    const int &nodeId2) {
    int minNodeId = qMin(nodeId1, nodeId2);
    NodeSettingsSPtr settings = mNodeSettings.at(minNodeId);
    settings->endEnabled = mNodeSettings.at(qMax(nodeId1, nodeId2))->endEnabled;
    PathContainer::mergeNodes(nodeId1, nodeId2);
    setCtrlsModeForNode(minNodeId, getNodeCtrlsMode(minNodeId));
    foreach(const KeySPtr &key, anim_mKeys) {
        SPtrGetAs(key, PathKey)->mergeNodes(nodeId1, nodeId2);
    }

    setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame, false));
    prp_updateInfluenceRangeAfterChanged();
}
