// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->getValue().actionRemoveNode(nodeId, approx);
    }
    mBaseValue.actionRemoveNode(nodeId, approx);
    prp_afterWholeInfluenceRangeChanged();
}

int SmartPathAnimator::actionAddNewAtStart(const QPointF &relPos) {
    return actionAddNewAtStart({false, false, CtrlsMode::corner,
                                relPos, relPos, relPos});
}

int SmartPathAnimator::actionAddNewAtStart(const NormalNodeData &data) {
    if(mBaseValue.getNodeCount() == 0)
        return actionAddFirstNode(data);
    beforeBinaryPathChange();
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->getValue().actionPrependNode();
    }
    const int id = mBaseValue.actionPrependNode();
    getCurrentlyEditedPath()->actionSetNormalNodeValues(id, data);
    prp_afterWholeInfluenceRangeChanged();
    return id;
}

int SmartPathAnimator::actionAddNewAtEnd(const QPointF &relPos) {
    return actionAddNewAtEnd({false, false, CtrlsMode::corner,
                              relPos, relPos, relPos});
}

int SmartPathAnimator::actionAddNewAtEnd(const NormalNodeData &data) {
    if(mBaseValue.getNodeCount() == 0)
        return actionAddFirstNode(data);
    beforeBinaryPathChange();
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->getValue().actionAppendNodeAtEndNode();
    }
    const int id = mBaseValue.actionAppendNodeAtEndNode();
    getCurrentlyEditedPath()->actionSetNormalNodeValues(id, data);
    prp_afterWholeInfluenceRangeChanged();
    return id;
}

int SmartPathAnimator::actionInsertNodeBetween(
        const int node1Id, const int node2Id, const qreal t) {
    beforeBinaryPathChange();
    const auto curr = getCurrentlyEditedPath();
    if(curr->getNodePtr(node1Id)->getCtrlsMode() == CtrlsMode::symmetric) {
        curr->actionSetNormalNodeCtrlsMode(node1Id, CtrlsMode::smooth);
    }
    if(curr->getNodePtr(node2Id)->getCtrlsMode() == CtrlsMode::symmetric) {
        curr->actionSetNormalNodeCtrlsMode(node2Id, CtrlsMode::smooth);
    }
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->getValue().actionInsertNodeBetween(node1Id, node2Id, t);
    }
    const int id = mBaseValue.actionInsertNodeBetween(node1Id, node2Id, t);
    curr->actionPromoteDissolvedNodeToNormal(id);
    prp_afterWholeInfluenceRangeChanged();
    return id;
}

void SmartPathAnimator::actionConnectNodes(
        const int node1Id, const int node2Id) {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->getValue().actionConnectNodes(node1Id, node2Id);
    }
    mBaseValue.actionConnectNodes(node1Id, node2Id);
    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionMergeNodes(const int node1Id, const int node2Id) {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->getValue().actionMergeNodes(node1Id, node2Id);
    }
    mBaseValue.actionMergeNodes(node1Id, node2Id);
    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionMoveNodeBetween(
        const int nodeId, const int prevNodeId, const int nextNodeId) {
    beforeBinaryPathChange();
    getCurrentlyEditedPath()->actionMoveNodeBetween(
                nodeId, prevNodeId, nextNodeId);
    pathChanged();
}

void SmartPathAnimator::actionClose() {
    actionConnectNodes(0, mBaseValue.getNodeCount() - 1);
}

void SmartPathAnimator::actionDisconnectNodes(const int node1Id,
                                              const int node2Id) {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        auto& keyPath = spKey->getValue();
        keyPath.actionDisconnectNodes(node1Id, node2Id);
    }
    mBaseValue.actionDisconnectNodes(node1Id, node2Id);
    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionReverseCurrent() {
    beforeBinaryPathChange();
    getCurrentlyEditedPath()->actionReversePath();
    pathChanged();
}

void SmartPathAnimator::actionReverseAll() {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->getValue().actionReversePath();
    }
    mBaseValue.actionReversePath();
    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionAppendMoveAllFrom(SmartPathAnimator * const other) {
    anim_coordinateKeysWith(other);
    const auto& keys = anim_getKeys();
    for(int i = 0; i < keys.count(); i++) {
        const auto thisKey = anim_getKeyAtIndex<SmartPathKey>(i);
        const auto otherKey = other->anim_getKeyAtIndex<SmartPathKey>(i);
        thisKey->getValue().actionAppendMoveAllFrom(
                    std::move(otherKey->getValue()));
    }
    mBaseValue.actionAppendMoveAllFrom(std::move(other->getBaseValue()));
    prp_afterWholeInfluenceRangeChanged();
    other->prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionPrependMoveAllFrom(SmartPathAnimator * const other) {
    anim_coordinateKeysWith(other);
    const auto& keys = anim_getKeys();
    for(int i = 0; i < keys.count(); i++) {
        const auto thisKey = anim_getKeyAtIndex<SmartPathKey>(i);
        const auto otherKey = other->anim_getKeyAtIndex<SmartPathKey>(i);
        thisKey->getValue().actionPrependMoveAllFrom(
                    std::move(otherKey->getValue()));
    }
    mBaseValue.actionPrependMoveAllFrom(std::move(other->getBaseValue()));
    prp_afterWholeInfluenceRangeChanged();
    other->prp_afterWholeInfluenceRangeChanged();
}

int SmartPathAnimator::actionAddFirstNode(const QPointF &relPos) {
    return actionAddFirstNode({false, false, CtrlsMode::symmetric,
                               relPos, relPos, relPos});
}

int SmartPathAnimator::actionAddFirstNode(const NormalNodeData &data) {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->getValue().actionAddFirstNode(data);
    }
    const int id = mBaseValue.actionAddFirstNode(data);
    prp_afterWholeInfluenceRangeChanged();
    return id;
}
