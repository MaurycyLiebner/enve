#include "vectorpathanimator.h"
#include "Animators/pathanimator.h"
#include "Boxes/boundingbox.h"
#include "edge.h"
#include "nodepoint.h"
#include "canvas.h"

VectorPathAnimator::VectorPathAnimator(PathAnimator *pathAnimator) :
    Animator() {
    prp_setName("path");
    mParentPathAnimator = pathAnimator;
}

VectorPathAnimator::VectorPathAnimator(const QList<NodeSettings*> &settingsList,
                                       PathAnimator *pathAnimator) :
    VectorPathAnimator(pathAnimator) {
    mNodeSettings = settingsList;
}

VectorPathAnimator::VectorPathAnimator(const QList<NodeSettings*> &settingsList,
                                       const QList<SkPoint> &posList,
                                       PathAnimator *pathAnimator) :
    VectorPathAnimator(settingsList,
                       pathAnimator) {
    mElementsPos = posList;
    updatePath();
    updateNodePointsFromElements();
}

VectorPathAnimator::~VectorPathAnimator() {
    foreach(NodePoint *pt, mPoints) {
        delete pt;
    }
    mPoints.clear();
    foreach(NodeSettings *settings, mNodeSettings) {
        delete settings;
    }
    mNodeSettings.clear();
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

void VectorPathAnimator::anim_saveCurrentValueToKey(PathKey *key) {
    key->setElementsFromSkPath(getPath());
}

void VectorPathAnimator::startPathChange() {
    if(mPathChanged) return;
    mPathChanged = true;
    if(prp_isRecording()) {
        if(prp_isKeyOnCurrentFrame()) return;
        anim_saveCurrentValueAsKey();
    }
}

void VectorPathAnimator::cancelPathChange() {
    mPathChanged = false;
}

void VectorPathAnimator::finishedPathChange() {
    if(mPathChanged) {
        mPathChanged = false;
    }
}

void VectorPathAnimator::setElementPos(const int &index,
                                       const SkPoint &pos) {
    if(prp_isKeyOnCurrentFrame()) {
        ((PathKey*)anim_mKeyOnCurrentFrame)->setElementPos(index, pos);
        anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
    } else {
        PathContainer::setElementPos(index, pos);
        prp_updateInfluenceRangeAfterChanged();
    }
}

void VectorPathAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) prp_setRecording(true);

    if(anim_mKeyOnCurrentFrame == NULL) {
        anim_mKeyOnCurrentFrame = new PathKey(anim_mCurrentRelFrame,
                                              getPath(),
                                              mElementsPos,
                                              this,
                                              mPathClosed);
        anim_appendKey(anim_mKeyOnCurrentFrame,
                       true,
                       false);
    } else {
        anim_saveCurrentValueToKey((PathKey*)anim_mKeyOnCurrentFrame);
    }
}

SkPath VectorPathAnimator::getPathAtRelFrame(const int &relFrame,
                                             const bool &considerCurrent) {
    //if(relFrame == anim_mCurrentRelFrame && considerCurrent) return getPath();
    SkPath pathToRuturn;
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(&prevId, &nextId, relFrame) ) {
        if(prevId == nextId) {
            pathToRuturn = ((PathKey*)anim_mKeys.at(prevId).get())->getPath();
        } else {
            PathKey *prevKey = ((PathKey*)anim_mKeys.at(prevId).get());
            PathKey *nextKey = ((PathKey*)anim_mKeys.at(nextId).get());
            int prevRelFrame = prevKey->getRelFrame();
            int nextRelFrame = nextKey->getRelFrame();
            SkScalar weight = ((SkScalar)relFrame - prevRelFrame)/
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
            setElementsFromSkPath(pathToRuturn);
        }
    }

    return pathToRuturn;
}

void VectorPathAnimator::anim_removeKey(Key *keyToRemove,
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

void VectorPathAnimator::anim_appendKey(Key *newKey,
                                        const bool &saveUndoRedo,
                                        const bool &update) {
    Animator::anim_appendKey(newKey, saveUndoRedo, update);
    mElementsUpdateNeeded = true;
}

VectorPathEdge *VectorPathAnimator::getEdge(const QPointF &absPos,
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

bool VectorPathAnimator::getTAndPointsForMouseEdgeInteraction(
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

    foreach(NodePoint *nodePoint, mPoints) {
        VectorPathEdge *edgeT = nodePoint->getNextEdge();
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
            *prevPoint = nodePoint;
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

NodePoint *VectorPathAnimator::createNewPointOnLineNear(
                                    const QPointF &absPos,
                                    const bool &adjust,
                                    const qreal &canvasScaleInv) {
    qreal pressedT;
    NodePoint *prevPoint = NULL;
    NodePoint *nextPoint = NULL;
    if(getTAndPointsForMouseEdgeInteraction(absPos, &pressedT,
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

            
            NodePoint *newPoint = new NodePoint(this);
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
                addNewPointAtTBetweenPts(pressedT,
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
                foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
                    ((PathKey*)key.get())->
                            addNewPointAtTBetweenPts(pressedT,
                                                     prevPtId,
                                                     nextPtId,
                                                     newPtSmooth);
                }
            }
            setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame));
            return newPoint;
        }
    }
    return NULL;
}

void VectorPathAnimator::updateNodePointsFromElements() {
    if(mFirstPoint != NULL) {
        mFirstPoint->setPointAsPrevious(NULL); // make sure path not closed
    }
    NodePoint *currOldNode = mFirstPoint;
    NodePoint *newFirstPt = NULL;
    int elementsCount = mElementsPos.count();
    int nodesCount = elementsCount/3;
    if(nodesCount == 0) return;
    NodePoint *lastP = NULL;
    for(int i = 0; i < nodesCount; i++) {
        int nodePtId = nodeIdToPointId(i);
        NodePoint *newP;
        if(currOldNode == NULL) {
            newP = new NodePoint(this);
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

    while(currOldNode != NULL) {
        NodePoint *oldOldNode = currOldNode;
        currOldNode = currOldNode->getNextPoint();
        mPoints.removeOne(oldOldNode);
        delete oldOldNode;
    }
    if(mPathClosed) {
        newFirstPt->setPointAsPrevious(lastP);
    } else {
        lastP->setPointAsNext(NULL);
    }
    setFirstPoint(newFirstPt);
}

void VectorPathAnimator::removeNodeAtAndApproximate(const int &nodeId) {
    if(nodeId <= 0 || nodeId >= mElementsPos.count()/3) return;

    setNodeCtrlsMode(nodeId + 1, CtrlsMode::CTRLS_CORNER);
    setNodeCtrlsMode(nodeId - 1, CtrlsMode::CTRLS_CORNER);
    PathContainer::removeNodeAtAndApproximate(nodeId);
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((PathKey*)key.get())->removeNodeAtAndApproximate(nodeId);
    }
}

void VectorPathAnimator::removeNodeAt(const int &nodeId) {
    PathContainer::removeNodeAt(nodeId);
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((PathKey*)key.get())->removeNodeAt(nodeId);
    }
    mElementsUpdateNeeded = true;
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
    Q_FOREACH(NodePoint *point, mPoints) {
        canvas->addPointToSelection(point);
    }
}

void VectorPathAnimator::applyTransformToPoints(const QMatrix &transform) {
    PathContainer::applyTransformToPoints(transform);
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((PathKey*)key.get())->applyTransformToPoints(transform);
    }
    updateNodePointsFromElements();
}

MovablePoint *VectorPathAnimator::getPointAtAbsPos(
                        const QPointF &absPtPos,
                        const CanvasMode &currentCanvasMode,
                        const qreal &canvasScaleInv) {
    Q_FOREACH(NodePoint *point, mPoints) {
        MovablePoint *pointToReturn =
                point->getPointAtAbsPos(absPtPos,
                                        currentCanvasMode,
                                        canvasScaleInv);
        if(pointToReturn == NULL) continue;
        return pointToReturn;
    }
    return NULL;
}

void VectorPathAnimator::selectAndAddContainedPointsToList(
                                const QRectF &absRect,
                                QList<MovablePoint *> *list) {
    Q_FOREACH(NodePoint *point, mPoints) {
        point->rectPointsSelection(absRect, list);
    }
}

void VectorPathAnimator::startAllPointsTransform() {
    Q_FOREACH(NodePoint *point, mPoints) {
        point->startTransform();
    }
}

void VectorPathAnimator::finishAllPointsTransform() {
    Q_FOREACH(NodePoint *point, mPoints) {
        point->finishTransform();
    }
}

void VectorPathAnimator::drawSelected(SkCanvas *canvas,
                                      const CanvasMode &currentCanvasMode,
                                      const qreal &invScale,
                                      const SkMatrix &combinedTransform) {

    if(currentCanvasMode == CanvasMode::MOVE_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            NodePoint *point = mPoints.at(i);
            point->drawSk(canvas, currentCanvasMode, invScale,
                          prp_isKeyOnCurrentFrame());
        }
    } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            NodePoint *point = mPoints.at(i);
            if(point->isEndPoint() || point->isSelected()) {
                point->drawSk(canvas, currentCanvasMode, invScale,
                              prp_isKeyOnCurrentFrame());
            }
        }
    }
}

void VectorPathAnimator::connectPoints(NodePoint *pt1,
                                       NodePoint *pt2) {
    pt1->connectToPoint(pt2);
    setPathClosed(true);
}

void VectorPathAnimator::appendToPointsList(NodePoint *pt) {
    mPoints << pt;
}

void VectorPathAnimator::setFirstPoint(NodePoint *firstPt) {
    if(mFirstPoint != NULL) {
        mFirstPoint->setSeparateNodePoint(false);
    }
    mFirstPoint = firstPt;
    if(mFirstPoint != NULL) {
        mFirstPoint->setSeparateNodePoint(true);
    }
}

void VectorPathAnimator::moveElementPosSubset(int firstId,
                                              int count,
                                              int targetId) {
    PathContainer::moveElementPosSubset(firstId, count, targetId);
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((PathKey*)key.get())->moveElementPosSubset(firstId, count, targetId);
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

void VectorPathAnimator::connectWith(VectorPathAnimator *srcPath) {
    QList<int> keyFrames;
    QList<QList<SkPoint> > newKeysData;
    QList<NodeSettings*> newNodeSettings;
    foreach(NodeSettings *setting, mNodeSettings) {
        newNodeSettings << setting;
    }
    QList<NodeSettings*> srcNodeSettings;
    srcPath->getNodeSettingsList(&srcNodeSettings);
    foreach(NodeSettings *setting, srcNodeSettings) {
        newNodeSettings << setting;
    }

    getKeysDataForConnection(this, srcPath, &keyFrames, &newKeysData, false);
    getKeysDataForConnection(srcPath, this, &keyFrames, &newKeysData, true);

    QList<SkPoint> srcElements;
    srcPath->getElementPosList(&srcElements);
    QList<SkPoint> defPosList;
    defPosList.append(mElementsPos);
    defPosList.append(srcElements);

    VectorPathAnimator *newAnimator = new VectorPathAnimator(newNodeSettings,
                                                             defPosList,
                                                             mParentPathAnimator);
    int idT = 0;
    foreach(const QList<SkPoint> &posList, newKeysData) {
        int relFrame = keyFrames.at(idT);
        PathKey *newKey = new PathKey(relFrame, posList, newAnimator, false);
        newAnimator->anim_appendKey(newKey);
        idT++;
    }

    mParentPathAnimator->addSinglePathAnimator(newAnimator);


    mParentPathAnimator->removeSinglePathAnimator(this);
    mParentPathAnimator->removeSinglePathAnimator(srcPath);
}

void VectorPathAnimator::revertElementPosSubset(
        const int &firstId,
        int count) {
    PathContainer::revertElementPosSubset(firstId, count);
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((PathKey*)key.get())->revertElementPosSubset(firstId, count);
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
        VectorPathAnimator *newSinglePath;
        QList<NodeSettings*> nodeSettings;
        {
            int firstNodeForNew = nextPt->getNodeId();
            int countNds = mNodeSettings.count() - firstNodeForNew;

            for(int i = 0; i < countNds; i++) {
                nodeSettings.append(mNodeSettings.takeAt(firstNodeForNew));
            }
        }
        if(prp_hasKeys()) {
            newSinglePath = new VectorPathAnimator(nodeSettings,
                                                   mParentPathAnimator);
            foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
                ((PathKey*)key.get())->createNewKeyFromSubsetForPath(
                                             newSinglePath,
                                             nextPt->getPtId() - 1,
                                             -1);
            }
            mElementsUpdateNeeded = true;
        } else {
            QList<SkPoint> elementsPos =
                    takeElementsPosSubset(nextPt->getPtId() - 1, -1);


            newSinglePath = new VectorPathAnimator(nodeSettings,
                                                   elementsPos,
                                                   mParentPathAnimator);
            mPathUpdateNeeded = true;
            updateNodePointsFromElements();
        }

        mParentPathAnimator->addSinglePathAnimator(newSinglePath);

        prp_updateInfluenceRangeAfterChanged();
    }
}

NodePoint *VectorPathAnimator::createNewNode(const int &targetNodeId,
                                             const QPointF &startRelPos,
                                             const QPointF &relPos,
                                             const QPointF &endRelPos,
                                             const NodeSettings &nodeSettings) {
    NodeSettings *nodeSettingsPtr =
            insertNodeSettingsForNodeId(targetNodeId,
                                        nodeSettings);
    int nodePtId = nodeIdToPointId(targetNodeId) - 1;
    insertElementPos(nodePtId, QPointFToSkPoint(endRelPos - relPos));
    insertElementPos(nodePtId, QPointFToSkPoint(relPos));
    insertElementPos(nodePtId, QPointFToSkPoint(startRelPos - relPos));
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((PathKey*)key.get())->insertElementPos(
                    nodePtId,
                    QPointFToSkPoint(endRelPos - relPos));
        ((PathKey*)key.get())->insertElementPos(
                    nodePtId,
                    QPointFToSkPoint(relPos));
        ((PathKey*)key.get())->insertElementPos(
                    nodePtId,
                    QPointFToSkPoint(startRelPos - relPos));
    }

    NodePoint *newP = new NodePoint(this);
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
        const NodeSettings &nodeSettings) {
    int targetNodeId;
    bool changeFirstPt = false;
    if(targetPt == NULL) {
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

    NodePoint *newP = createNewNode(targetNodeId,
                                    startRelPos, relPos, endRelPos,
                                    nodeSettings);

    if(changeFirstPt) {
        setFirstPoint(newP);
    }
    if(targetPt != NULL) {
        targetPt->connectToPoint(newP);
    }
    updateNodePointIds();
    prp_updateInfluenceRangeAfterChanged();
    return newP;
}

void VectorPathAnimator::updateNodePointIds() {
    if(mFirstPoint == NULL) return;
    int pointId = 0;
    NodePoint *nextPoint = mFirstPoint;
    while(true) {
        nextPoint->setNodeId(pointId);
        pointId++;
        nextPoint = nextPoint->getNextPoint();
        if(nextPoint == NULL || nextPoint == mFirstPoint) break;
    }
}

void VectorPathAnimator::shiftAllPointsForAllKeys(const int &by) {
    PathContainer::shiftAllPoints(by);
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((PathKey*)key.get())->shiftAllPoints(by);
    }
    setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame, false));
}

void VectorPathAnimator::revertAllPointsForAllKeys() {
    PathContainer::revertAllPoints();
    foreach(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((PathKey*)key.get())->revertAllPoints();
    }
    setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame, false));
}
