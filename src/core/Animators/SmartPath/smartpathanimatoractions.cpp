// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "smartpathanimator.h"

void SmartPathAnimator::actionRemoveNode(const int nodeId, const bool approx) {
    prp_pushUndoRedoName("Remove Node");

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        key->startValueTransform();
        spKey->getValue().actionRemoveNode(nodeId, approx);
        key->finishValueTransform();
    }
    prp_startTransform();
    mBaseValue.actionRemoveNode(nodeId, approx);
    prp_finishTransform();

    if(mBaseValue.isEmpty()) emit emptied();
    else prp_afterWholeInfluenceRangeChanged();
}

int SmartPathAnimator::actionAddNewAtStart(const QPointF &relPos) {
    return actionAddNewAtStart({false, false, CtrlsMode::corner,
                                relPos, relPos, relPos});
}

int SmartPathAnimator::actionAddNewAtStart(const NormalNodeData &data) {
    if(mBaseValue.getNodeCount() == 0)
        return actionAddFirstNode(data);

    prp_pushUndoRedoName("Add New Node");

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->startValueTransform();
        spKey->getValue().actionPrependNode();
        spKey->finishValueTransform();
    }

    prp_startTransform();
    const int id = mBaseValue.actionPrependNode();
    getCurrentlyEditedPath()->actionSetNormalNodeValues(id, data);
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    return id;
}

int SmartPathAnimator::actionAddNewAtEnd(const QPointF &relPos) {
    return actionAddNewAtEnd({false, false, CtrlsMode::corner,
                              relPos, relPos, relPos});
}

int SmartPathAnimator::actionAddNewAtEnd(const NormalNodeData &data) {
    if(mBaseValue.getNodeCount() == 0) return actionAddFirstNode(data);
    prp_pushUndoRedoName("Add New Node");

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->startValueTransform();
        spKey->getValue().actionAppendNodeAtEndNode();
        spKey->finishValueTransform();
    }

    prp_startTransform();
    const int id = mBaseValue.actionAppendNodeAtEndNode();
    getCurrentlyEditedPath()->actionSetNormalNodeValues(id, data);
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    return id;
}

int SmartPathAnimator::actionInsertNodeBetween(
        const int node1Id, const int node2Id, const qreal t) {
    prp_pushUndoRedoName("Add New Node");

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->startValueTransform();
        spKey->getValue().actionInsertNodeBetween(node1Id, node2Id, t);
        spKey->finishValueTransform();
    }

    prp_startTransform();
    const auto curr = getCurrentlyEditedPath();
    if(curr->getNodePtr(node1Id)->getCtrlsMode() == CtrlsMode::symmetric) {
        curr->actionSetNormalNodeCtrlsMode(node1Id, CtrlsMode::smooth);
    }
    if(curr->getNodePtr(node2Id)->getCtrlsMode() == CtrlsMode::symmetric) {
        curr->actionSetNormalNodeCtrlsMode(node2Id, CtrlsMode::smooth);
    }
    const int id = mBaseValue.actionInsertNodeBetween(node1Id, node2Id, t);
    curr->actionPromoteDissolvedNodeToNormal(id);
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    return id;
}

void SmartPathAnimator::actionConnectNodes(
        const int node1Id, const int node2Id) {
    prp_pushUndoRedoName("Connect Nodes");

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->startValueTransform();
        spKey->getValue().actionConnectNodes(node1Id, node2Id);
        spKey->finishValueTransform();
    }

    prp_startTransform();
    mBaseValue.actionConnectNodes(node1Id, node2Id);
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionMergeNodes(const int node1Id, const int node2Id) {
    prp_pushUndoRedoName("Merge Nodes");

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->startValueTransform();
        spKey->getValue().actionMergeNodes(node1Id, node2Id);
        spKey->finishValueTransform();
    }

    prp_startTransform();
    mBaseValue.actionMergeNodes(node1Id, node2Id);
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionMoveNodeBetween(
        const int nodeId, const int prevNodeId, const int nextNodeId) {
    prp_pushUndoRedoName("Move Node");
    prp_startTransform();
    getCurrentlyEditedPath()->actionMoveNodeBetween(
                nodeId, prevNodeId, nextNodeId);
    prp_finishTransform();
    pathChanged();
}

void SmartPathAnimator::actionClose() {
    actionConnectNodes(0, mBaseValue.getNodeCount() - 1);
}

void SmartPathAnimator::actionDisconnectNodes(const int node1Id,
                                              const int node2Id) {
    prp_pushUndoRedoName("Disconnect Nodes");

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        auto& keyPath = spKey->getValue();
        key->startValueTransform();
        keyPath.actionDisconnectNodes(node1Id, node2Id);
        key->finishValueTransform();
    }

    prp_startTransform();
    mBaseValue.actionDisconnectNodes(node1Id, node2Id);
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionReverseCurrent() {
    prp_pushUndoRedoName("Reverse Nodes");

    prp_startTransform();
    getCurrentlyEditedPath()->actionReversePath();
    prp_finishTransform();
    pathChanged();
}

void SmartPathAnimator::actionReverseAll() {
    prp_pushUndoRedoName("Reverse Nodes");

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        key->startValueTransform();
        spKey->getValue().actionReversePath();
        key->finishValueTransform();
    }

    prp_startTransform();
    mBaseValue.actionReversePath();
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionAppendMoveAllFrom(SmartPathAnimator * const other) {
    prp_pushUndoRedoName("Connect Paths");

    anim_coordinateKeysWith(other);
    const auto& keys = anim_getKeys();
    for(int i = 0; i < keys.count(); i++) {
        const auto thisKey = anim_getKeyAtIndex<SmartPathKey>(i);
        const auto otherKey = other->anim_getKeyAtIndex<SmartPathKey>(i);

        thisKey->startValueTransform();
        thisKey->getValue().actionAppendMoveAllFrom(
                    std::move(otherKey->getValue()));
        thisKey->finishValueTransform();
    }

    prp_startTransform();
    mBaseValue.actionAppendMoveAllFrom(std::move(other->getBaseValue()));
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    other->prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionPrependMoveAllFrom(SmartPathAnimator * const other) {
    prp_pushUndoRedoName("Connect Paths");

    anim_coordinateKeysWith(other);
    const auto& keys = anim_getKeys();
    for(int i = 0; i < keys.count(); i++) {
        const auto thisKey = anim_getKeyAtIndex<SmartPathKey>(i);
        const auto otherKey = other->anim_getKeyAtIndex<SmartPathKey>(i);
        thisKey->startValueTransform();
        thisKey->getValue().actionPrependMoveAllFrom(
                    std::move(otherKey->getValue()));
        thisKey->finishValueTransform();
    }

    prp_startTransform();
    mBaseValue.actionPrependMoveAllFrom(std::move(other->getBaseValue()));
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    other->prp_afterWholeInfluenceRangeChanged();
}

int SmartPathAnimator::actionAddFirstNode(const QPointF &relPos) {
    return actionAddFirstNode({false, false, CtrlsMode::symmetric,
                               relPos, relPos, relPos});
}

int SmartPathAnimator::actionAddFirstNode(const NormalNodeData &data) {
    prp_pushUndoRedoName("Add New Node");

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        key->startValueTransform();
        spKey->getValue().actionAddFirstNode(data);
        key->finishValueTransform();
    }

    prp_startTransform();
    const int id = mBaseValue.actionAddFirstNode(data);
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    return id;
}
