#include "vectorpathanimator.h"
#include "Animators/pathanimator.h"
#include "Boxes/boundingbox.h"
#include "edge.h"
#include "MovablePoints/nodepoint.h"
#include "canvas.h"
#include "undoredo.h"
#include "Animators/transformanimator.h"

VectorPathAnimator::VectorPathAnimator(PathAnimator *pathAnimator) :
    InterpolationAnimator("path") {
    setParentPath(pathAnimator);
}

VectorPathAnimator::VectorPathAnimator(
        const QList<const NodeSettings *> &settingsList,
        PathAnimator *pathAnimator) :
    VectorPathAnimator(pathAnimator) {
    for(const auto& nodeSetting : settingsList) {
        mNodeSettings << SPtrCreate(NodeSettings)(nodeSetting);
    }
}

VectorPathAnimator::VectorPathAnimator(
        const QList<const NodeSettings *> &settingsList,
        const QList<SkPoint> &posList,
        PathAnimator *pathAnimator)  :
    VectorPathAnimator(settingsList, pathAnimator) {
    mElementsPos = posList;
    updatePath();
    updateNodePointsFromElements();
}

void VectorPathAnimator::anim_setAbsFrame(const int &frame) {
    Animator::anim_setAbsFrame(frame);
    //setCurrentPath(getPathAtRelFrame(frame));
    if(anim_hasKeys()) {
        setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame));
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
    NodeSettings* newSettings = mNodeSettings.at(nodeId).get();
    newSettings->ctrlsMode = mode;
    newSettings->endEnabled = true;
    newSettings->startEnabled = true;
    int nodePtId = nodeIdToPointId(nodeId);
    QPointF pos = skPointToQ(mElementsPos.at(nodePtId));
    QPointF startPos = skPointToQ(mElementsPos.at(nodePtId - 1)) + pos;
    QPointF endPos = skPointToQ(mElementsPos.at(nodePtId + 1)) + pos;
    QPointF newStartPos;
    QPointF newEndPos;
    if(mode == CtrlsMode::CTRLS_SYMMETRIC) {
        gGetCtrlsSymmetricPos(endPos, startPos, pos,
                              newEndPos, newStartPos);
        mElementsPos.replace(nodePtId - 1, qPointToSk(newStartPos - pos));
        mElementsPos.replace(nodePtId + 1, qPointToSk(newEndPos));
    } else if(mode == CtrlsMode::CTRLS_SMOOTH) {
        gGetCtrlsSmoothPos(endPos, startPos, pos,
                           newEndPos, newStartPos);
        mElementsPos.replace(nodePtId - 1, qPointToSk(newStartPos - pos));
    }

    //mParentPath->schedulePathUpdate();
}

void VectorPathAnimator::revertAllNodeSettings() {
    for(const auto& settings : mNodeSettings) {
        bool endT = settings->endEnabled;
        settings->endEnabled = settings->startEnabled;
        settings->startEnabled = endT;
    }
}

void VectorPathAnimator::startPathChange() {
    if(mPathChanged) return;
    if(anim_isRecording()) {
        if(anim_isKeyOnCurrentFrame()) return;
        anim_saveCurrentValueAsKey();
    }
    if(anim_isKeyOnCurrentFrame()) {
        GetAsPK(anim_mKeyOnCurrentFrame)->startPathChange();
        anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
    } else {
        PathContainer::startPathChange();
        prp_updateInfluenceRangeAfterChanged();
    }
    mPathChanged = true;
}

void VectorPathAnimator::cancelPathChange() {
    if(!mPathChanged) return;
    if(anim_isKeyOnCurrentFrame()) {
        GetAsPK(anim_mKeyOnCurrentFrame)->cancelPathChange();
        anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
    } else {
        PathContainer::cancelPathChange();
        prp_updateInfluenceRangeAfterChanged();
    }
    mPathChanged = false;
}

void VectorPathAnimator::finishedPathChange() {
    if(!mPathChanged) return;
    if(anim_isKeyOnCurrentFrame()) {
        GetAsPK(anim_mKeyOnCurrentFrame)->finishedPathChange();
        anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
    } else {
        PathContainer::finishedPathChange();
        prp_updateInfluenceRangeAfterChanged();
    }
    mPathChanged = false;
    prp_callFinishUpdater();
}

void VectorPathAnimator::setElementPos(const int &index,
                                       const SkPoint &pos) {
    if(anim_isKeyOnCurrentFrame()) {
        GetAsPK(anim_mKeyOnCurrentFrame)->setElementPos(index, pos);
        anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
    } else {
        PathContainer::setElementPos(index, pos);
        prp_updateInfluenceRangeAfterChanged();
    }
}

void VectorPathAnimator::anim_addKeyAtRelFrame(const int& relFrame) {
    if(!anim_mIsRecording) anim_setRecording(true);

    auto newKey = GetAsSPtr(anim_getKeyAtRelFrame(relFrame), PathKey);

    if(!newKey) {
        newKey = SPtrCreate(PathKey)(relFrame, getPathAtRelFrame(relFrame),
                                     this, mPathClosed);
        anim_appendKey(newKey);
    } else {
        newKey->setElementsFromSkPath(getPathAtRelFrame(relFrame));
    }
}

void VectorPathAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) anim_setRecording(true);

    if(anim_mKeyOnCurrentFrame == nullptr) {
        auto newKey = SPtrCreate(PathKey)(anim_mCurrentRelFrame,
                                          getPath(),
                                          mElementsPos,
                                          this,
                                          mPathClosed);
        anim_appendKey(newKey);
        anim_mKeyOnCurrentFrame = newKey.get();
    } else {
        anim_saveCurrentValueToKey(GetAsPK(anim_mKeyOnCurrentFrame));
    }
}

SkPath VectorPathAnimator::getPathAtRelFrame(const int &relFrame) {
    if(mElementsUpdateNeeded) finalizeNodesRemove();
    //if(relFrame == anim_mCurrentRelFrame && considerCurrent) return getPath();
    SkPath pathToRuturn;
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(prevId, nextId, relFrame) ) {
        if(prevId == nextId) {
            pathToRuturn = GetAsPK(anim_mKeys.at(prevId))->getPath();
        } else {
            const auto prevKey = GetAsPK(anim_mKeys.at(prevId));
            const auto nextKey = GetAsPK(anim_mKeys.at(nextId));
            int prevRelFrame = prevKey->getRelFrame();
            int nextRelFrame = nextKey->getRelFrame();
            const qreal intRelFrame = getInterpolatedFrameAtRelFrameF(relFrame);
            const qreal weight = (intRelFrame - prevRelFrame)/
                    (nextRelFrame - prevRelFrame);
            nextKey->getPath().interpolate(
                        prevKey->getPath(),
                        qrealToSkScalar(weight),
                        &pathToRuturn);
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

SkPath VectorPathAnimator::getPathAtRelFrameF(const qreal &relFrame) {
    if(mElementsUpdateNeeded) finalizeNodesRemove();
    SkPath pathToRuturn;
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrameF(prevId, nextId, relFrame) ) {
        if(prevId == nextId) {
            pathToRuturn = GetAsPK(anim_mKeys.at(prevId))->getPath();
        } else {
            const auto prevKey = GetAsPK(anim_mKeys.at(prevId));
            const auto nextKey = GetAsPK(anim_mKeys.at(nextId));
            int prevRelFrame = prevKey->getRelFrame();
            int nextRelFrame = nextKey->getRelFrame();
            const qreal intRelFrame = getInterpolatedFrameAtRelFrameF(relFrame);
            const qreal weight = (intRelFrame - prevRelFrame)/
                                 (nextRelFrame - prevRelFrame);
            nextKey->getPath().interpolate(prevKey->getPath(),
                                           qrealToSkScalar(weight),
                                           &pathToRuturn);
        }
    } else {
        pathToRuturn = getPath();
    }

    if(mElementsUpdateNeeded) {
        if(isZero4Dec(relFrame - anim_mCurrentRelFrame)) {
            mElementsUpdateNeeded = false;
            finalizeNodesRemove();
            setElementsFromSkPath(pathToRuturn);
        }
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

void VectorPathAnimator::anim_removeKey(const stdsptr<Key>& keyToRemove) {
    Animator::anim_removeKey(keyToRemove);
    mElementsUpdateNeeded = true;
    //    if(anim_mKeys.count() == 0) {
    //        setElementsFromSkPath(getPath());
    //    }
}

void VectorPathAnimator::anim_moveKeyToRelFrame(Key *key, const int &newFrame) {
    Animator::anim_moveKeyToRelFrame(key, newFrame);
    mElementsUpdateNeeded = true;
}

void VectorPathAnimator::anim_appendKey(const stdsptr<Key>& newKey) {
    Animator::anim_appendKey(newKey);
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
    const QMatrix &combinedTransform = mParentTransform->getCombinedTransform();
    const qreal xScaling = combinedTransform.map(
                        QLineF(0, 0, 1, 0)).length();
    const qreal yScaling = combinedTransform.map(
                        QLineF(0, 0, 0, 1)).length();
    const qreal maxDistX = 4/xScaling*canvasScaleInv;
    const qreal maxDistY = 4/yScaling*canvasScaleInv;
    const QPointF relPos = combinedTransform.inverted().map(absPos);

//    if(!mPath.intersects(distRect) ||
//        mPath.contains(distRect)) {
//        return false;
//    }

    QPointF nearestPos = relPos + QPointF(maxDistX, maxDistY)*2;
    qreal nearestT = 0.5;
    qreal nearestError = 1000000.;

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

            
            const auto newPoint = createNodePointAndAppendToList();

            stdsptr<NodeSettings> newNodeSettings;
            if(adjust) {
                if(!prevNodeSettings->endEnabled &&
                   !nextNodeSettings->startEnabled) {
                    newNodeSettings = SPtrCreate(NodeSettings)(false, false,
                                                               CTRLS_CORNER);
                } else {
                    newPtSmooth = true;
                    newNodeSettings = SPtrCreate(NodeSettings)(true, true,
                                                               CTRLS_SMOOTH);
                    if(prevNodeSettings->ctrlsMode == CTRLS_SYMMETRIC &&
                       prevNodeSettings->endEnabled &&
                       prevNodeSettings->startEnabled) {
                        prevNodeSettings->set(true, true, CTRLS_SMOOTH);
                    }
                    if(nextNodeSettings->ctrlsMode == CTRLS_SYMMETRIC &&
                       nextNodeSettings->endEnabled &&
                       nextNodeSettings->startEnabled) {
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
                                         prevPtId,
                                         nextPtId,
                                         newPtSmooth);
                nextPtId = nodeIdToPointId(nextNodeId);
                prevPoint->setElementsPos(skPointToQ(mElementsPos.at(prevPtId - 1)),
                                         skPointToQ(mElementsPos.at(prevPtId)),
                                         skPointToQ(mElementsPos.at(prevPtId + 1)));
                newPoint->setElementsPos(skPointToQ(mElementsPos.at(prevPtId + 2)),
                                         skPointToQ(mElementsPos.at(prevPtId + 3)),
                                         skPointToQ(mElementsPos.at(prevPtId + 4)));
                nextPoint->setElementsPos(skPointToQ(mElementsPos.at(nextPtId - 1)),
                                         skPointToQ(mElementsPos.at(nextPtId)),
                                         skPointToQ(mElementsPos.at(nextPtId + 1)));
            } else {
                for(const auto &key : anim_mKeys) {
                    GetAsPK(key)->
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
        newP->setElementsPos(skPointToQ(mElementsPos.at(nodePtId - 1)),
                             skPointToQ(mElementsPos.at(nodePtId)),
                             skPointToQ(mElementsPos.at(nodePtId + 1)));
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
    settings->startEnabled = enabled;
}

void VectorPathAnimator::setNodeEndEnabled(const int &nodeId,
                                           const bool &enabled) {
    const auto settings = getNodeSettingsForNodeId(nodeId);
    settings->endEnabled = enabled;
}

void VectorPathAnimator::setNodeCtrlsMode(const int &nodeId,
                                          const CtrlsMode &ctrlsMode) {
    const auto settings = getNodeSettingsForNodeId(nodeId);
    settings->ctrlsMode = ctrlsMode;
}

void VectorPathAnimator::setPathClosed(const bool &bT) {
    PathContainer::setPathClosed(bT);
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->setPathClosed(bT);
    }
    if(anim_hasKeys()) {
        setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame));
    }
    prp_updateInfluenceRangeAfterChanged();
}

const CtrlsMode &VectorPathAnimator::getNodeCtrlsMode(const int &nodeId) {
    return getNodeSettingsForNodeId(nodeId)->ctrlsMode;
}

void VectorPathAnimator::removeNodeAt(const int &nodeId) {
    if(nodeId < 0 || nodeId >= mElementsPos.count()/3) return;
    mNodesToRemove.append(nodeId);
    mElementsUpdateNeeded = true;
    prp_updateInfluenceRangeAfterChanged();
}

void VectorPathAnimator::selectAllPoints(Canvas *canvas) {
    for(const auto& point : mPoints) {
        canvas->addPointToSelection(point.get());
    }
}

void VectorPathAnimator::applyTransformToPoints(const QMatrix &transform) {
    PathContainer::applyTransformToPoints(transform);
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->applyTransformToPoints(transform);
    }
    updateNodePointsFromElements();
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
                                      const SkMatrix &combinedTransform) {
    Q_UNUSED(combinedTransform);

    const bool keyOnCurrentFrame = anim_isKeyOnCurrentFrame();
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

VectorPathAnimator * VectorPathAnimator::connectWith(VectorPathAnimator *srcPath) {
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
        updateNodePointsFromElements();
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
                                             const bool& startEnabled,
                                             const bool& endEnabled,
                                             const CtrlsMode& ctrlsMode) {
    auto newNodeSettings = SPtrCreate(NodeSettings)(startEnabled,
                                                    endEnabled,
                                                    ctrlsMode);
    insertNodeSettingsForNodeId(targetNodeId, newNodeSettings);

    int nodePtId = nodeIdToPointId(targetNodeId) - 1;
    addNodeElements(nodePtId,
                    qPointToSk(startRelPos - relPos),
                    qPointToSk(relPos),
                    qPointToSk(endRelPos - relPos));
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->addNodeElements(nodePtId,
                        qPointToSk(startRelPos - relPos),
                        qPointToSk(relPos),
                        qPointToSk(endRelPos - relPos));
    }

    auto newP = createNodePointAndAppendToList();

    newP->setCurrentNodeSettings(newNodeSettings.get());
    newP->setElementsPos(startRelPos - relPos,
                         relPos,
                         endRelPos - relPos);
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

    if(changeFirstPt) {
        setFirstPoint(newP);
    }
    if(targetPt) {
        targetPt->connectToPoint(newP);
    }
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
        if(targetPtId == -1) {
            ptT = mPoints.last().get();
        } else {
            ptT = mPoints.at(targetPtId).get();
        }
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
    setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame));
}

void VectorPathAnimator::revertAllPointsForAllKeys() {
    PathContainer::revertAllPoints();
    revertAllNodeSettings();
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->revertAllPoints();
    }
    setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame));
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
    if(anim_mIsRecording) {
        anim_saveCurrentValueAsKey();
    } else {
        updateNodePointsFromElements();
    }
}

void VectorPathAnimator::revertAllPoints() {
    PathContainer::revertAllPoints();
    revertAllNodeSettings();
    if(anim_mIsRecording) {
        anim_saveCurrentValueAsKey();
    } else {
        updateNodePointsFromElements();
    }
}

void VectorPathAnimator::updateAfterChangedFromInside() {
    prp_updateInfluenceRangeAfterChanged();
}

void VectorPathAnimator::removeFromParent() {
    if(!mParentPathAnimator) return;
    mParentPathAnimator->removeSinglePathAnimator(ref<VectorPathAnimator>());
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
    settings->endEnabled = mNodeSettings.at(qMax(nodeId1, nodeId2))->endEnabled;
    PathContainer::mergeNodes(nodeId1, nodeId2);
    setCtrlsModeForNode(minNodeId, getNodeCtrlsMode(minNodeId));
    for(const auto &key : anim_mKeys) {
        GetAsPK(key)->mergeNodes(nodeId1, nodeId2);
    }

    setElementsFromSkPath(getPathAtRelFrame(anim_mCurrentRelFrame));
    prp_updateInfluenceRangeAfterChanged();
}

const bool &VectorPathAnimator::isClosed() const {
    return mPathClosed;
}
