#include "vectorpathanimator.h"
#include "Animators/pathanimator.h"
#include "Boxes/boundingbox.h"
#include "edge.h"
#include "MovablePoints/nodepoint.h"
#include "canvas.h"
#include "undoredo.h"
#include "Animators/transformanimator.h"

VectorPathAnimator::VectorPathAnimator(PathAnimator * const pathAnimator) :
    InterpolationAnimator("path") {
    setParentPath(pathAnimator);
}

VectorPathAnimator::VectorPathAnimator(
        const QList<const NodeSettings*> &settingsList,
        PathAnimator * const pathAnimator) :
    VectorPathAnimator(pathAnimator) {
    for(const auto& nodeSetting : settingsList) {
        mNodeSettings << SPtrCreate(NodeSettings)(nodeSetting);
    }
}

VectorPathAnimator::VectorPathAnimator(
        const QList<const NodeSettings*> &settingsList,
        const QList<SkPoint> &posList,
        PathAnimator * const pathAnimator)  :
    VectorPathAnimator(settingsList, pathAnimator) {
    mElementsPos = posList;
    updatePathFromData();
    updateNodePointsFromData();
}

void VectorPathAnimator::anim_setAbsFrame(const int &frame) {
    const int lastPrevFrame = anim_getCurrentRelFrame();
    Animator::anim_setAbsFrame(frame);
    //setCurrentPath(getPathAtRelFrame(frame));
    if(prp_differencesBetweenRelFrames(lastPrevFrame, frame))
        setElementsFromSkPath(getPathAtRelFrame(anim_getCurrentRelFrame()));
}

void VectorPathAnimator::setElementsFromSkPath(const SkPath &path) {
    PathContainer::setElementsFromSkPath(path);
    updateNodePointsFromData();
}

void VectorPathAnimator::anim_saveCurrentValueToKey(PathKey* key) {
    key->setElementsFromSkPath(getPath());
}

void VectorPathAnimator::setCtrlsModeForNode(const int &nodeId,
                                             const CtrlsMode &mode) {
    NodeSettings* newSettings = mNodeSettings.at(nodeId).get();
    newSettings->fCtrlsMode = mode;
    newSettings->fEndEnabled = true;
    newSettings->fStartEnabled = true;
    const int nodePtId = nodeIdToPointId(nodeId);
    const QPointF pos = toQPointF(mElementsPos.at(nodePtId));
    const QPointF startPos = toQPointF(mElementsPos.at(nodePtId - 1)) + pos;
    const QPointF endPos = toQPointF(mElementsPos.at(nodePtId + 1)) + pos;
    QPointF newStartPos;
    QPointF newEndPos;
    if(mode == CtrlsMode::CTRLS_SYMMETRIC) {
        gGetCtrlsSymmetricPos(startPos, pos, endPos,
                              newStartPos, newEndPos);
        mElementsPos.replace(nodePtId - 1, toSkPoint(newStartPos - pos));
        mElementsPos.replace(nodePtId + 1, toSkPoint(newEndPos));
    } else if(mode == CtrlsMode::CTRLS_SMOOTH) {
        gGetCtrlsSmoothPos(startPos, pos, endPos,
                           newStartPos, newEndPos);
        mElementsPos.replace(nodePtId - 1, toSkPoint(newStartPos - pos));
    }

    //mParentPath->schedulePathUpdate();
}

void VectorPathAnimator::revertAllNodeSettings() {
    for(const auto& settings : mNodeSettings) {
        const bool endT = settings->fEndEnabled;
        settings->fEndEnabled = settings->fStartEnabled;
        settings->fStartEnabled = endT;
    }
}

void VectorPathAnimator::startPathChange() {
    if(mPathChanged) return;
    if(anim_isRecording()) {
        if(anim_getKeyOnCurrentFrame()) return;
        anim_saveCurrentValueAsKey();
    }
    const auto currKey = anim_getKeyOnCurrentFrame<PathKey>();
    if(currKey) {
        currKey->startPathChange();
        anim_updateAfterChangedKey(currKey);
    } else {
        PathContainer::startPathChange();
        prp_updateInfluenceRangeAfterChanged();
    }
    mPathChanged = true;
}

void VectorPathAnimator::cancelPathChange() {
    if(!mPathChanged) return;
    const auto currKey = anim_getKeyOnCurrentFrame<PathKey>();
    if(currKey) {
        currKey->cancelPathChange();
        anim_updateAfterChangedKey(currKey);
    } else {
        PathContainer::cancelPathChange();
        prp_updateInfluenceRangeAfterChanged();
    }
    mPathChanged = false;
}

void VectorPathAnimator::finishedPathChange() {
    if(!mPathChanged) return;
    const auto currKey = anim_getKeyOnCurrentFrame<PathKey>();
    if(currKey) {
        currKey->finishedPathChange();
        anim_updateAfterChangedKey(currKey);
    } else {
        PathContainer::finishedPathChange();
        prp_updateInfluenceRangeAfterChanged();
    }
    mPathChanged = false;
    prp_callFinishUpdater();
}

void VectorPathAnimator::setElementPos(const int &index, const SkPoint &pos) {
    const auto currKey = anim_getKeyOnCurrentFrame<PathKey>();
    if(currKey) {
        currKey->setElementPos(index, pos);
        anim_updateAfterChangedKey(currKey);
    } else {
        PathContainer::setElementPos(index, pos);
        prp_updateInfluenceRangeAfterChanged();
    }
}

void VectorPathAnimator::anim_addKeyAtRelFrame(const int& relFrame) {
    if(anim_getKeyAtRelFrame(relFrame)) return;
    const auto newKey = SPtrCreate(PathKey)(
                relFrame, getPathAtRelFrame(relFrame), this, mPathClosed);
    anim_appendKey(newKey);
}

void VectorPathAnimator::anim_saveCurrentValueAsKey() {
    if(anim_getKeyOnCurrentFrame()) return;
    auto newKey = SPtrCreate(PathKey)(anim_getCurrentRelFrame(),
                                      getPath(), mElementsPos,
                                      this, mPathClosed);
    anim_appendKey(newKey);
}

SkPath VectorPathAnimator::getPathAtRelFrame(const qreal &relFrame) {
    SkPath pathToRuturn;
    const auto pn = anim_getPrevAndNextKeyIdForRelFrameF(relFrame);
    const int prevId = pn.first;
    const int nextId = pn.second;

    const auto prevKey = anim_getKeyAtIndex<PathKey>(prevId);
    const auto nextKey = anim_getKeyAtIndex<PathKey>(nextId);
    const bool adjKeys = pn.second - pn.first == 1;
    const auto keyAtRelFrame = adjKeys ? nullptr :
                               anim_getKeyAtIndex<PathKey>(pn.first + 1);
    if(keyAtRelFrame) {
        pathToRuturn = keyAtRelFrame->getPath();
    } else if(!prevKey && ! nextKey) {
        pathToRuturn = getPath();
    } else if(!prevKey) {
        pathToRuturn = nextKey->getPath();
    } else if(!nextKey) {
        pathToRuturn = prevKey->getPath();
    } else { // if(nextKey && prevKey) {
        const int prevRelFrame = prevKey->getRelFrame();
        const int nextRelFrame = nextKey->getRelFrame();
        const qreal intRelFrame = getInterpolatedFrameAtRelFrame(relFrame);
        const qreal weight = (intRelFrame - prevRelFrame)/
                             (nextRelFrame - prevRelFrame);
        nextKey->getPath().interpolate(prevKey->getPath(),
                                       toSkScalar(weight),
                                       &pathToRuturn);
    }

    return pathToRuturn;
}

NodeSettings *VectorPathAnimator::getNodeSettingsForPtId(const int &ptId) {
    return mNodeSettings.at(pointIdToNodeId(ptId)).get();
}

NodeSettings *VectorPathAnimator::getNodeSettingsForNodeId(const int &nodeId) {
    return mNodeSettings.at(nodeId).get();
}

void VectorPathAnimator::replaceNodeSettingsForPtId(
        const int &ptId, const NodeSettings *settings) {
    replaceNodeSettingsForNodeId(pointIdToNodeId(ptId), settings);
}

void VectorPathAnimator::replaceNodeSettingsForNodeId(
        const int &nodeId, const NodeSettings *settings) {
    mNodeSettings.at(nodeId)->copyFrom(settings);
}

NodeSettings* VectorPathAnimator::insertNodeSettingsForNodeId(
        const int &nodeId,
        const NodeSettings* settings) {
    auto newSettings = SPtrCreate(NodeSettings)(settings);
    insertNodeSettingsForNodeId(nodeId, newSettings);
    return newSettings.get();
}

void VectorPathAnimator::insertNodeSettingsForNodeId(
        const int &nodeId,
        const stdsptr<NodeSettings>& newSettings) {
    mNodeSettings.insert(nodeId, newSettings);
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
            const auto edge = prevPoint->getNextEdge();
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
    const QMatrix &totalTransform = mParentTransform->getTotalTransform();
    const qreal xScaling = totalTransform.map(
                        QLineF(0, 0, 1, 0)).length();
    const qreal yScaling = totalTransform.map(
                        QLineF(0, 0, 0, 1)).length();
    const qreal maxDistX = 4/xScaling*canvasScaleInv;
    const qreal maxDistY = 4/yScaling*canvasScaleInv;
    const QPointF relPos = totalTransform.inverted().map(absPos);

//    if(!mPath.intersects(distRect) ||
//        mPath.contains(distRect)) {
//        return false;
//    }

    QPointF nearestPos = relPos + QPointF(maxDistX, maxDistY)*2;
    qreal nearestT = 0.5;
    qreal nearestError = TEN_MIL;

    for(const auto& nodePoint : mPoints) {
        const auto edgeT = nodePoint->getNextEdge();
        if(!edgeT) continue;
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
    const QPointF distT = nearestPos - relPos;
    if(qAbs(distT.x()) > maxDistX ||
       qAbs(distT.y()) > maxDistY) return false;

    pressedT = nearestT;
    if(*prevPoint == nullptr) return false;

    *nextPoint = (*prevPoint)->getNextPoint();
    if(*nextPoint == nullptr) return false;

    return true;
}

NodePoint* VectorPathAnimator::createNodePointAndAppendToList() {
    auto nodePt = SPtrCreate(NodePoint)(this, mParentTransform);
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
            const int prevNodeId = prevPoint->getNodeId();
            const int newNodeId = prevNodeId + 1;

            NodeSettings * const prevNodeSettings =
                    getNodeSettingsForNodeId(prevNodeId);
            NodeSettings *nextNodeSettings;
            int nextNodeId;
            const int prevPtId = nodeIdToPointId(prevNodeId);
            int nextPtId;
            if(nextPoint->getNodeId() == 0) {
                nextNodeSettings = getNodeSettingsForNodeId(0);
                nextNodeId = 0;
                nextPtId = 1;
            } else {
                nextNodeSettings = getNodeSettingsForNodeId(prevNodeId + 1);
                nextPtId = nodeIdToPointId(prevNodeId + 1);
                nextNodeId = prevNodeId + 2;
            }

            bool newPtSmooth = false;

            
            const auto newPoint = createNodePointAndAppendToList();

            stdsptr<NodeSettings> newNodeSettings;
            if(adjust) {
                if(!prevNodeSettings->fEndEnabled &&
                   !nextNodeSettings->fStartEnabled) {
                    newNodeSettings = SPtrCreate(NodeSettings)(false, false,
                                                               CTRLS_CORNER);
                } else {
                    newPtSmooth = true;
                    newNodeSettings = SPtrCreate(NodeSettings)(true, true,
                                                               CTRLS_SMOOTH);
                    if(prevNodeSettings->fCtrlsMode == CTRLS_SYMMETRIC &&
                       prevNodeSettings->fEndEnabled &&
                       prevNodeSettings->fStartEnabled) {
                        prevNodeSettings->set(true, true, CTRLS_SMOOTH);
                    }
                    if(nextNodeSettings->fCtrlsMode == CTRLS_SYMMETRIC &&
                       nextNodeSettings->fEndEnabled &&
                       nextNodeSettings->fStartEnabled) {
                        nextNodeSettings->set(true, true, CTRLS_SMOOTH);
                    }
                }
            } else {
                newNodeSettings = SPtrCreate(NodeSettings)(false, false,
                                                           CTRLS_CORNER);
            }
            insertNodeSettingsForNodeId(newNodeId, newNodeSettings);

            newPoint->setCurrentNodeSettings(newNodeSettings.get());

            nextPoint->setPointAsPrevious(newPoint);
            prevPoint->setPointAsNext(newPoint);
            updateNodePointIds();

            if(anim_mKeys.isEmpty()) {
                addNewPointAtTBetweenPts(static_cast<SkScalar>(pressedT),
                                         prevPtId, nextPtId, newPtSmooth);
                nextPtId = nodeIdToPointId(nextNodeId);
                prevPoint->setElementsPos(toQPointF(mElementsPos.at(prevPtId - 1)),
                                         toQPointF(mElementsPos.at(prevPtId)),
                                         toQPointF(mElementsPos.at(prevPtId + 1)));
                newPoint->setElementsPos(toQPointF(mElementsPos.at(prevPtId + 2)),
                                         toQPointF(mElementsPos.at(prevPtId + 3)),
                                         toQPointF(mElementsPos.at(prevPtId + 4)));
                nextPoint->setElementsPos(toQPointF(mElementsPos.at(nextPtId - 1)),
                                         toQPointF(mElementsPos.at(nextPtId)),
                                         toQPointF(mElementsPos.at(nextPtId + 1)));
            } else {
                for(const auto &key : anim_mKeys) {
                    GetAsPK(key)->
                            addNewPointAtTBetweenPts(static_cast<SkScalar>(pressedT),
                                                     prevPtId,
                                                     nextPtId,
                                                     newPtSmooth);
                }
            }
            setElementsFromSkPath(getPathAtRelFrame(anim_getCurrentRelFrame()));
            return newPoint;
        }
    }
    return nullptr;
}

void VectorPathAnimator::updateNodePointsFromData() {
    if(mElementsUpdateNeeded) {
        mElementsUpdateNeeded = false;
        finalizeNodesRemove();
        setElementsFromSkPath(getPath());
    }
    if(mFirstPoint) {
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
        if(!currOldNode) {
            newP = createNodePointAndAppendToList();
        } else {
            newP = currOldNode;
            currOldNode = currOldNode->getNextPoint();
        }
        newP->setCurrentNodeSettings(getNodeSettingsForNodeId(i));
        newP->setElementsPos(toQPointF(mElementsPos.at(nodePtId - 1)),
                             toQPointF(mElementsPos.at(nodePtId)),
                             toQPointF(mElementsPos.at(nodePtId + 1)));
        newP->setNodeId(i);

        if(i == 0) {
            newFirstPt = newP;
        } else {
            newP->setPointAsPrevious(lastP);
        }
        lastP = newP;
    }

    Canvas* parentCanvas = nullptr;
    if(mParentPathAnimator) {
        const auto parentBox = mParentPathAnimator->getParentBox();
        if(parentBox) parentCanvas = parentBox->getParentCanvas();
    }
    while(currOldNode) {
        const auto oldOldNode = currOldNode;
        currOldNode = currOldNode->getNextPoint();
        if(oldOldNode->isSelected()) {
            if(parentCanvas) {
                parentCanvas->removePointFromSelection(oldOldNode);
            }
        }
        mPoints.removeOne(GetAsSPtr(oldOldNode, NodePoint));
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
    qSort(mNodesToRemove);
    for(int i = mNodesToRemove.count() - 1; i >= 0; i--) {
        const int &nodeId = mNodesToRemove.at(i);
        removeNodeSettingsAt(nodeId);
        PathContainer::removeNodeAt(nodeId);
        for(const auto &key : anim_mKeys) {
            GetAsPK(key)->removeNodeAt(nodeId);
        }
    }
    mNodesToRemove.clear();
}

void VectorPathAnimator::removeNodeAtAndApproximate(const int &nodeId) {
    if(nodeId <= 0 || nodeId >= mElementsPos.count()/3) return;
    removeNodeSettingsAt(nodeId);

    setNodeCtrlsMode(nodeId + 1, CtrlsMode::CTRLS_CORNER);
    setNodeCtrlsMode(nodeId - 1, CtrlsMode::CTRLS_CORNER);
    PathContainer::removeNodeAtAndApproximate(nodeId);
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->removeNodeAtAndApproximate(nodeId);
    }
    mElementsUpdateNeeded = true;
}

void VectorPathAnimator::removeNodeSettingsAt(const int &id) {
    mNodeSettings.removeAt(id);
}

void VectorPathAnimator::setNodeStartEnabled(const int &nodeId,
                                             const bool &enabled) {
    const auto settings = getNodeSettingsForNodeId(nodeId);
    settings->fStartEnabled = enabled;
}

void VectorPathAnimator::setNodeEndEnabled(const int &nodeId,
                                           const bool &enabled) {
    const auto settings = getNodeSettingsForNodeId(nodeId);
    settings->fEndEnabled = enabled;
}

void VectorPathAnimator::setNodeCtrlsMode(const int &nodeId,
                                          const CtrlsMode &ctrlsMode) {
    const auto settings = getNodeSettingsForNodeId(nodeId);
    settings->fCtrlsMode = ctrlsMode;
}

void VectorPathAnimator::setPathClosed(const bool &bT) {
    PathContainer::setPathClosed(bT);
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->setPathClosed(bT);
    }
    if(anim_hasKeys()) {
        setElementsFromSkPath(getPathAtRelFrame(anim_getCurrentRelFrame()));
    }
    prp_updateInfluenceRangeAfterChanged();
}

const CtrlsMode &VectorPathAnimator::getNodeCtrlsMode(const int &nodeId) {
    return getNodeSettingsForNodeId(nodeId)->fCtrlsMode;
}

void VectorPathAnimator::removeNodeAt(const int &nodeId) {
    if(nodeId < 0 || nodeId >= mElementsPos.count()/3) return;
    mNodesToRemove.append(nodeId);
    mElementsUpdateNeeded = true;
    prp_updateInfluenceRangeAfterChanged();
}

void VectorPathAnimator::selectAllPoints(Canvas * const canvas) {
    for(const auto& point : mPoints) {
        canvas->addPointToSelection(point.get());
    }
}

void VectorPathAnimator::applyTransformToPoints(const QMatrix &transform) {
    PathContainer::applyTransformToPoints(transform);
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->applyTransformToPoints(transform);
    }
    updateNodePointsFromData();
}

MovablePoint *VectorPathAnimator::getPointAtAbsPos(
                        const QPointF &absPtPos,
                        const CanvasMode &currentCanvasMode,
                        const qreal &canvasScaleInv) {
    for(const auto& point : mPoints) {
        auto pointToReturn = point->getPointAtAbsPos(absPtPos,
                                                     currentCanvasMode,
                                                     canvasScaleInv);
        if(!pointToReturn) continue;
        return pointToReturn;
    }
    return nullptr;
}

void VectorPathAnimator::setParentPath(PathAnimator *parentPath) {
    auto newParentBox = parentPath ? parentPath->getParentBox() : nullptr;
    auto oldParentBox = mParentPathAnimator ?
                mParentPathAnimator->getParentBox() : nullptr;

    if(newParentBox != oldParentBox) {
        mParentTransform = newParentBox ?
                    newParentBox->getTransformAnimator() : nullptr;
        for(const auto& point : mPoints) {
            point->setParentTransformAnimator(mParentTransform);
        }
    }
    mParentPathAnimator = parentPath;
}

void VectorPathAnimator::selectAndAddContainedPointsToList(
        const QRectF &absRect, QList<stdptr<MovablePoint>> &list) {
    for(const auto& point : mPoints) {
        point->rectPointsSelection(absRect, list);
    }
}

void VectorPathAnimator::startAllPointsTransform() {
    for(const auto& point : mPoints) {
        point->startTransform();
    }
}

void VectorPathAnimator::finishAllPointsTransform() {
    for(const auto& point : mPoints) {
        point->finishTransform();
    }
}

void VectorPathAnimator::drawSelected(SkCanvas *canvas,
                                      const CanvasMode &currentCanvasMode,
                                      const SkScalar &invScale,
                                      const SkMatrix &totalTransform) {
    Q_UNUSED(totalTransform);

    const bool keyOnCurrentFrame = anim_getKeyOnCurrentFrame();
    if(currentCanvasMode == CanvasMode::MOVE_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            auto point = getNodePtWithNodeId(i);
            point->drawNodePoint(canvas, currentCanvasMode, invScale,
                                 keyOnCurrentFrame);
        }
    } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
        for(int i = mPoints.count() - 1; i >= 0; i--) {
            auto point = getNodePtWithNodeId(i);
            if(point->isEndPoint() || point->isSelected()) {
                point->drawNodePoint(canvas, currentCanvasMode, invScale,
                                     keyOnCurrentFrame);
            }
        }
    }
}

void VectorPathAnimator::connectPoints(NodePoint *pt1,
                                       NodePoint* pt2) {
    pt1->connectToPoint(pt2);
    setPathClosed(true);
}

void VectorPathAnimator::appendToPointsList(const stdsptr<NodePoint>& pt) {
    mPoints << pt;
}

void VectorPathAnimator::setFirstPoint(NodePoint* pt) {
    if(mFirstPoint) mFirstPoint->setSeparateNodePoint(false);
    mFirstPoint = pt;
    if(mFirstPoint) mFirstPoint->setSeparateNodePoint(true);
}

void VectorPathAnimator::moveElementPosSubset(int firstId,
                                              int count,
                                              int targetId) {
    PathContainer::moveElementPosSubset(firstId, count, targetId);
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->moveElementPosSubset(firstId, count, targetId);
    }
}

void VectorPathAnimator::revertNodeSettingsSubset(const int &firstId,
                                                  int count) {
    if(count == -1) count = mNodeSettings.count() - firstId;
    int lastId = firstId + count - 1;
    for(int i = 0; i < count; i++) {
        mNodeSettings.move(lastId, firstId + i);
    }
}

void VectorPathAnimator::getNodeSettingsList(
        QList<stdsptr<NodeSettings>> &nodeSettingsList) {
    nodeSettingsList = mNodeSettings;
}

const QList<stdsptr<NodeSettings>> &VectorPathAnimator::getNodeSettingsList() {
    return mNodeSettings;
}

void VectorPathAnimator::getElementPosList(QList<SkPoint> *elementPosList) {
    *elementPosList = mElementsPos;
}

void VectorPathAnimator::getKeysList(QList<stdsptr<PathKey>> &pathKeyList) {
    for(const auto &key : anim_mKeys) {
        pathKeyList.append(GetAsSPtr(key, PathKey));
    }
}

void VectorPathAnimator::getKeysDataForConnection(
        VectorPathAnimator *targetPath, VectorPathAnimator *srcPath,
        QList<int> &keyFrames, QList<QList<SkPoint>> &newKeysData,
        const bool &addSrcFirst) {
    QList<stdsptr<PathKey>> keys;
    srcPath->getKeysList(keys);
    for(const auto& srcKey : keys) {
        QList<SkPoint> combinedKeyData;
        int relFrame = srcKey->getRelFrame();
        Key* keyAtRelFrame = targetPath->anim_getKeyAtRelFrame(relFrame);
        const QList<SkPoint> &srcKeyElements = srcKey->getElementsPosList();
        QList<SkPoint> targetKeyElements;
        if(!keyAtRelFrame) {
            targetKeyElements = extractElementsFromSkPath(
                        targetPath->getPathAtRelFrame(relFrame));
        } else {
            targetKeyElements =
                    GetAsPK(keyAtRelFrame)->getElementsPosList();
        }
        if(addSrcFirst) {
            combinedKeyData.append(targetKeyElements);
            combinedKeyData.append(srcKeyElements);
        } else {
            combinedKeyData.append(targetKeyElements);
            combinedKeyData.append(srcKeyElements);
        }
        newKeysData.append(combinedKeyData);
        keyFrames.append(srcKey->getRelFrame());
    }
}

VectorPathAnimator * VectorPathAnimator::connectWith(
        VectorPathAnimator *srcPath) {
    QList<int> keyFrames;
    QList<QList<SkPoint> > newKeysData;
    QList<const NodeSettings*> newNodeSettings;
    for(const auto& setting : mNodeSettings) {
        newNodeSettings << setting.get();
    }
    QList<stdsptr<NodeSettings>> srcNodeSettings;
    srcPath->getNodeSettingsList(srcNodeSettings);
    for(const auto& setting : srcNodeSettings) {
        newNodeSettings << setting.get();
    }

    getKeysDataForConnection(this, srcPath, keyFrames, newKeysData, false);
    getKeysDataForConnection(srcPath, this, keyFrames, newKeysData, true);

    QList<SkPoint> srcElements;
    srcPath->getElementPosList(&srcElements);
    QList<SkPoint> defPosList;
    defPosList.append(mElementsPos);
    defPosList.append(srcElements);

    auto newAnimator = SPtrCreate(VectorPathAnimator)(
                newNodeSettings, defPosList, mParentPathAnimator);
    int idT = 0;
    for(const auto& posList : newKeysData) {
        int relFrame = keyFrames.at(idT);
        auto newKey = SPtrCreate(PathKey)(relFrame, posList,
                                          newAnimator.get(), false);
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
    for(const auto &key : anim_mKeys) {
        GetAsPK(key.get())->revertElementPosSubset(firstId, count);
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
        updateNodePointsFromData();
    } else {
        qsptr<VectorPathAnimator> newSinglePath;
        QList<const NodeSettings*> nodeSettings;
        {
            int firstNodeForNew = nextPt->getNodeId();
            int countNds = mNodeSettings.count() - firstNodeForNew;

            for(int i = 0; i < countNds; i++) {
                nodeSettings.append(mNodeSettings.at(firstNodeForNew).get());
                removeNodeSettingsAt(firstNodeForNew);
            }
        }
        if(anim_hasKeys()) {
            newSinglePath = SPtrCreate(VectorPathAnimator)(
                        nodeSettings, mParentPathAnimator);
            for(const auto &key : anim_mKeys) {
                GetAsPK(key)->createNewKeyFromSubsetForPath(
                                             newSinglePath.get(),
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
            updateNodePointsFromData();
        }

        mParentPathAnimator->addSinglePathAnimator(newSinglePath);

        prp_updateInfluenceRangeAfterChanged();
    }
}

NodePoint* VectorPathAnimator::createNewNode(const int &targetNodeId,
                                             const QPointF &startRelPos,
                                             const QPointF &relPos,
                                             const QPointF &endRelPos,
                                             const bool& startEnabled,
                                             const bool& endEnabled,
                                             const CtrlsMode& ctrlsMode) {
    auto newNodeSettings = SPtrCreate(NodeSettings)(startEnabled,
                                                    endEnabled,
                                                    ctrlsMode);
    insertNodeSettingsForNodeId(targetNodeId, newNodeSettings);

    const int nodePtId = nodeIdToPointId(targetNodeId) - 1;
    addNodeElements(nodePtId,
                    toSkPoint(startRelPos - relPos),
                    toSkPoint(relPos),
                    toSkPoint(endRelPos - relPos));
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->addNodeElements(nodePtId,
                        toSkPoint(startRelPos - relPos),
                        toSkPoint(relPos),
                        toSkPoint(endRelPos - relPos));
    }

    auto newP = createNodePointAndAppendToList();

    newP->setCurrentNodeSettings(newNodeSettings.get());
    newP->setElementsPos(startRelPos - relPos, relPos, endRelPos - relPos);
    return newP;
}

NodePoint *VectorPathAnimator::addNodeRelPos(
        const QPointF &startRelPos,
        const QPointF &relPos,
        const QPointF &endRelPos,
        const bool& startEnabled,
        const bool& endEnabled,
        const CtrlsMode& ctrlsMode,
        NodePoint *targetPt) {
    int targetNodeId;
    bool changeFirstPt = false;
    if(!targetPt) {
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

    auto newP = createNewNode(targetNodeId,
                              startRelPos, relPos, endRelPos,
                              startEnabled, endEnabled, ctrlsMode);

    if(changeFirstPt) setFirstPoint(newP);
    if(targetPt) targetPt->connectToPoint(newP);
    updateNodePointIds();
    prp_updateInfluenceRangeAfterChanged();

    return newP;
}
#include "svgimporter.h"

NodePoint *VectorPathAnimator::addNodeAbsPos(
        const QPointF &absPos, NodePoint *targetPt) {
    return addNodeRelPos(mParentTransform->mapAbsPosToRel(absPos), targetPt);
}

NodePoint *VectorPathAnimator::addNodeRelPos(
        const QPointF &relPos, NodePoint *targetPt) {
    return addNodeRelPos(relPos, relPos, relPos, targetPt);
}

NodePoint *VectorPathAnimator::addNodeRelPos(
        const QPointF &startRelPos,
        const QPointF &relPos,
        const QPointF &endRelPos,
        NodePoint *targetPt) {
    return addNodeRelPos(startRelPos, relPos, endRelPos,
                         false, false, CTRLS_CORNER, targetPt);
}

NodePoint *VectorPathAnimator::addNodeRelPos(
        const SvgNodePoint* svgPoint, NodePoint* targetPt) {
    return addNodeRelPos(svgPoint->getStartPoint(),
                         svgPoint->getPoint(),
                         svgPoint->getEndPoint(),
                         svgPoint->getStartPointEnabled(),
                         svgPoint->getEndPointEnabled(),
                         svgPoint->getCtrlsMode(),
                         targetPt);
}

NodePoint *VectorPathAnimator::addNodeRelPos(
        const QPointF &startRelPos,
        const QPointF &relPos,
        const QPointF &endRelPos,
        const bool& startEnabled,
        const bool& endEnabled,
        const CtrlsMode& ctrlsMode,
        const int &targetPtId) {
    NodePoint* ptT = nullptr;
    if(!mPoints.isEmpty()) {
        if(targetPtId == -1) ptT = mPoints.last().get();
        else ptT = mPoints.at(targetPtId).get();
    }
    return addNodeRelPos(startRelPos, relPos, endRelPos,
                         startEnabled, endEnabled, ctrlsMode,
                         ptT);
}

void VectorPathAnimator::updateNodePointIds() {
    if(!mFirstPoint) return;
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
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->shiftAllPoints(by);
    }
    setElementsFromSkPath(getPathAtRelFrame(anim_getCurrentRelFrame()));
}

void VectorPathAnimator::revertAllPointsForAllKeys() {
    PathContainer::revertAllPoints();
    revertAllNodeSettings();
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->revertAllPoints();
    }
    setElementsFromSkPath(getPathAtRelFrame(anim_getCurrentRelFrame()));
}

void VectorPathAnimator::shiftAllNodeSettings(const int &by) {
    if(by == 0) return;
    if(mPathClosed) {
        for(int i = 0; i < by*3; i++) {
            mNodeSettings.prepend(mNodeSettings.takeLast());
        }
        for(int i = 0; i < -by*3; i++) {
            mNodeSettings.append(mNodeSettings.takeFirst());
        }
        mPathUpdateNeeded = true;
    }
}

void VectorPathAnimator::shiftAllPoints(const int &by) {
    PathContainer::shiftAllPoints(by);
    shiftAllNodeSettings(by);
    if(anim_isRecording()) {
        anim_saveCurrentValueAsKey();
    } else {
        updateNodePointsFromData();
    }
}

void VectorPathAnimator::revertAllPoints() {
    PathContainer::revertAllPoints();
    revertAllNodeSettings();
    if(anim_isRecording()) {
        anim_saveCurrentValueAsKey();
    } else {
        updateNodePointsFromData();
    }
}

void VectorPathAnimator::updateAfterChangedFromInside() {
    prp_updateInfluenceRangeAfterChanged();
}

void VectorPathAnimator::removeFromParent() {
    if(!mParentPathAnimator) return;
    mParentPathAnimator->removeSinglePathAnimator_k(ref<VectorPathAnimator>());
}

NodePoint *VectorPathAnimator::getNodePtWithNodeId(const int &id) {
    if(id < 0 || id >= mPoints.count()) return nullptr;
    return mPoints.at(id).get();
}

int VectorPathAnimator::getNodeCount() {
    return mNodeSettings.count();
}

void VectorPathAnimator::mergeNodes(const int &nodeId1,
                                    const int &nodeId2) {
    int minNodeId = qMin(nodeId1, nodeId2);
    const auto settings = mNodeSettings.at(minNodeId);
    settings->fEndEnabled = mNodeSettings.at(qMax(nodeId1, nodeId2))->fEndEnabled;
    PathContainer::mergeNodes(nodeId1, nodeId2);
    setCtrlsModeForNode(minNodeId, getNodeCtrlsMode(minNodeId));
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->mergeNodes(nodeId1, nodeId2);
    }

    setElementsFromSkPath(getPathAtRelFrame(anim_getCurrentRelFrame()));
    prp_updateInfluenceRangeAfterChanged();
}

const bool &VectorPathAnimator::isClosed() const {
    return mPathClosed;
}
