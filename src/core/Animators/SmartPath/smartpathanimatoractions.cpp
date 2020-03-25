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

#include "node.h"

void SmartPathAnimator::removeNode(const int nodeId, const bool approx) {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->getValue().actionRemoveNode(nodeId, approx);
    }
    baseValue().actionRemoveNode(nodeId, approx);
}

void SmartPathAnimator::actionRemoveNode(const int nodeId, const bool approx) {
    prp_pushUndoRedoName("Remove Node");

    prp_startTransform();

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        key->startValueTransform();
    }
    removeNode(nodeId, approx);
    for(const auto &key : keys) {
        key->finishValueTransform();
    }
    prp_finishTransform();

    if(baseValue().isEmpty()) emit emptied();
    else prp_afterWholeInfluenceRangeChanged();
}

int SmartPathAnimator::actionAddNewAtStart(const QPointF &relPos) {
    return actionAddNewAtStart({false, false, CtrlsMode::corner,
                                relPos, relPos, relPos});
}

int SmartPathAnimator::actionAddNewAtStart(const NormalNodeData &data) {
    if(baseValue().getNodeCount() == 0)
        return actionAddFirstNode(data);

    prp_pushUndoRedoName("Add New Node");

    prp_startTransform();

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->startValueTransform();
        spKey->getValue().actionPrependNode();
        spKey->finishValueTransform();
    }

    const int id = baseValue().actionPrependNode();
    getCurrentlyEdited()->actionSetNormalNodeValues(id, data);
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    return id;
}

int SmartPathAnimator::actionAddNewAtEnd(const QPointF &relPos) {
    return actionAddNewAtEnd({false, false, CtrlsMode::corner,
                              relPos, relPos, relPos});
}

int SmartPathAnimator::actionAddNewAtEnd(const NormalNodeData &data) {
    if(baseValue().getNodeCount() == 0) return actionAddFirstNode(data);
    prp_pushUndoRedoName("Add New Node");

    prp_startTransform();

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->startValueTransform();
        spKey->getValue().actionAppendNodeAtEndNode();
        spKey->finishValueTransform();
    }

    const int id = baseValue().actionAppendNodeAtEndNode();
    getCurrentlyEdited()->actionSetNormalNodeValues(id, data);
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    return id;
}

int SmartPathAnimator::actionInsertNodeBetween(
        const int node1Id, const int node2Id,
        const qreal t, const NodePointValues& vals) {
    prp_pushUndoRedoName("Add New Node");

    prp_startTransform();
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->startValueTransform();
        spKey->getValue().actionInsertNodeBetween(node1Id, node2Id, t);
        spKey->finishValueTransform();
    }

    const auto curr = getCurrentlyEdited();
    if(&baseValue() != curr) {
        baseValue().actionInsertNodeBetween(node1Id, node2Id, t);
    }
    const int id = curr->actionInsertNodeBetween(node1Id, node2Id, vals);

    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    return id;
}

int SmartPathAnimator::insertNodeBetween(
        const int node1Id, const int node2Id, const qreal t) {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->getValue().actionInsertNodeBetween(node1Id, node2Id, t);
    }

    const auto curr = getCurrentlyEdited();
    if(curr->getNodePtr(node1Id)->getCtrlsMode() == CtrlsMode::symmetric) {
        curr->actionSetNormalNodeCtrlsMode(node1Id, CtrlsMode::smooth);
    }
    if(curr->getNodePtr(node2Id)->getCtrlsMode() == CtrlsMode::symmetric) {
        curr->actionSetNormalNodeCtrlsMode(node2Id, CtrlsMode::smooth);
    }
    const int id = baseValue().actionInsertNodeBetween(node1Id, node2Id, t);
    curr->actionPromoteDissolvedNodeToNormal(id);
    return id;
}

int SmartPathAnimator::actionInsertNodeBetween(
        const int node1Id, const int node2Id, const qreal t) {
    prp_pushUndoRedoName("Add New Node");

    prp_startTransform();
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        key->startValueTransform();
    }
    const int id = insertNodeBetween(node1Id, node2Id, t);
    for(const auto &key : keys) {
        key->finishValueTransform();
    }
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    return id;
}

void SmartPathAnimator::connectNodes(
        const int node1Id, const int node2Id) {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->getValue().actionConnectNodes(node1Id, node2Id);
    }

    baseValue().actionConnectNodes(node1Id, node2Id);
}

void SmartPathAnimator::actionConnectNodes(
        const int node1Id, const int node2Id) {
    prp_pushUndoRedoName("Connect Nodes");

    prp_startTransform();
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        key->startValueTransform();
    }

    connectNodes(node1Id, node2Id);

    for(const auto &key : keys) {
        key->finishValueTransform();
    }

    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionMergeNodes(const int node1Id, const int node2Id) {
    prp_pushUndoRedoName("Merge Nodes");

    prp_startTransform();
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        spKey->startValueTransform();
        spKey->getValue().actionMergeNodes(node1Id, node2Id);
        spKey->finishValueTransform();
    }

    baseValue().actionMergeNodes(node1Id, node2Id);
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionMoveNodeBetween(
        const int nodeId, const int prevNodeId, const int nextNodeId) {
    prp_pushUndoRedoName("Move Node");
    prp_startTransform();
    getCurrentlyEdited()->actionMoveNodeBetween(
                nodeId, prevNodeId, nextNodeId);
    prp_finishTransform();
    changed();
}

void SmartPathAnimator::close() {
    connectNodes(0, baseValue().getNodeCount() - 1);
}

void SmartPathAnimator::actionClose() {
    actionConnectNodes(0, baseValue().getNodeCount() - 1);
}

void SmartPathAnimator::actionDisconnectNodes(const int node1Id,
                                              const int node2Id) {
    prp_pushUndoRedoName("Disconnect Nodes");

    prp_startTransform();
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        auto& keyPath = spKey->getValue();
        key->startValueTransform();
        keyPath.actionDisconnectNodes(node1Id, node2Id);
        key->finishValueTransform();
    }

    baseValue().actionDisconnectNodes(node1Id, node2Id);
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionReverseCurrent() {
    prp_pushUndoRedoName("Reverse Nodes");

    prp_startTransform();
    getCurrentlyEdited()->actionReversePath();
    prp_finishTransform();
    changed();
}

void SmartPathAnimator::actionReverseAll() {
    prp_pushUndoRedoName("Reverse Nodes");

    prp_startTransform();

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        key->startValueTransform();
        spKey->getValue().actionReversePath();
        key->finishValueTransform();
    }

    baseValue().actionReversePath();
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionAppendMoveAllFrom(SmartPathAnimator * const other) {
    prp_pushUndoRedoName("Connect Paths");

    prp_startTransform();

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

    baseValue().actionAppendMoveAllFrom(std::move(other->baseValue()));
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    other->prp_afterWholeInfluenceRangeChanged();
}

void SmartPathAnimator::actionPrependMoveAllFrom(SmartPathAnimator * const other) {
    prp_pushUndoRedoName("Connect Paths");

    prp_startTransform();

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

    baseValue().actionPrependMoveAllFrom(std::move(other->baseValue()));
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    other->prp_afterWholeInfluenceRangeChanged();
}

class WrappedInt {
public:
    WrappedInt(const int value, const int wrapValue, const bool reverse) :
        mSign(reverse ? -1 : 1), mWrapVal(wrapValue) {
        setValue(value);
    }

    void setValue(const int value) {
        if(value < 0) {
            mVal = mWrapVal + value % mWrapVal;
        } else {
            mVal = value % mWrapVal;
        }
    }

    WrappedInt& operator+=(const int val) {
        setValue(mVal + mSign*val);
        return *this;
    }

    WrappedInt& operator-=(const int val) {
        setValue(mVal - mSign*val);
        return *this;
    }

    WrappedInt& operator++() {
        setValue(mVal + mSign);
        return *this;
    }

    WrappedInt& operator--() {
        setValue(mVal - mSign);
        return *this;
    }

    WrappedInt operator++(int) {
        WrappedInt result = *this;
        setValue(mVal + mSign);
        return result;
    }

    WrappedInt operator--(int) {
        WrappedInt result = *this;
        setValue(mVal - mSign);
        return result;
    }

    WrappedInt operator-(const int val) {
        return WrappedInt(mVal - val, mWrapVal, mSign == -1);
    }

    WrappedInt operator+(const int val) {
        return WrappedInt(mVal + val, mWrapVal, mSign == -1);
    }

    bool operator==(const int val) const {
        return mVal == val;
    }

    bool operator!=(const int val) const {
        return mVal != val;
    }

    void setWrapValue(const int wrapVal) {
        mWrapVal = wrapVal;
        setValue(mVal);
    }

    int toInt() const { return mVal; }
    bool isReverse() const { return mSign == -1; }
private:
    const int mSign;
    int mVal;
    int mWrapVal;
};

QVector<int> replaceNodeIds(const int count1, const int count2) {
    const int min = qMin(count1, count2);
    const int max = qMax(count1, count2);
    QVector<int> result;
    result.reserve(min);
    for(int i = 0; i < min; i++) {
        const qreal t = (1. + i)/(min + 1.);
        const bool secondHalf = t > 0.5;
        qreal targetF = (max - 1)*(secondHalf ? 1 - t : t);
        int target;
        if(isInteger4Dec(targetF) || isZero4Dec(t - 0.5)) {
            target = qRound(targetF);
        } else {
            target = qFloor(targetF);
            if(secondHalf) target = max - 1 - target;
        }
        result << target;
    }
    return result;
}

void SmartPathAnimator::actionReplaceSegments(
        int beginNodeId, int endNodeId,
        const QList<qCubicSegment2D>& with) {
    if(with.isEmpty() || beginNodeId == endNodeId) return;

    prp_pushUndoRedoName("Replace Nodes");

    const auto edit = getCurrentlyEdited();

    prp_startTransform();

    const bool reverse = endNodeId < beginNodeId;
//    const bool close = reverse && !isClosed();
    int totalCount = edit->getNodeCount();
    const int currentCount = reverse ? beginNodeId - endNodeId - 1 :
                                       endNodeId - beginNodeId - 1;
    const int replaceCount = with.count() - 1;
    const bool changeAll = replaceCount != currentCount;

    const auto replaceIds = replaceNodeIds(currentCount, replaceCount);

    const auto& keys = anim_getKeys();
    if(changeAll) {
        for(const auto &key : keys) {
            key->startValueTransform();
        }
    }

//    if(close) this->close();

    const auto& firstSeg = with.first();
    const auto& lastSeg = with.last();
    edit->actionSetNormalNodeCtrlsMode(beginNodeId, CtrlsMode::corner);
    edit->actionSetNormalNodeCtrlsMode(endNodeId, CtrlsMode::corner);
    if(reverse) {
        edit->actionSetNormalNodeC0(beginNodeId, firstSeg.c1());
        edit->actionSetNormalNodeC2(endNodeId, lastSeg.c2());
    } else {
        edit->actionSetNormalNodeC2(beginNodeId, firstSeg.c1());
        edit->actionSetNormalNodeC0(endNodeId, lastSeg.c2());
    }

    const auto edited = getCurrentlyEdited();

    const int iInc = reverse ? -1 : 1;
    for(WrappedInt i(beginNodeId + iInc, totalCount, reverse);
        i != endNodeId; i++) {
        const auto iNode = edited->getNodePtr(i.toInt());
        if(iNode->isDissolved()) {
            edited->actionPromoteDissolvedNodeToNormal(i.toInt());
        }
    }

    if(replaceCount < currentCount) {
        const bool remove = !anim_hasKeys();
        if(remove) {
            const int removeCount = currentCount - replaceCount;
            for(int i = 0; i < removeCount; i++) {
                const int idValue = beginNodeId + iInc;
                const int nodeId = WrappedInt(idValue, totalCount, reverse).toInt();
                removeNode(nodeId, false);
                totalCount--;
                if(beginNodeId > nodeId) beginNodeId--;
                if(endNodeId > nodeId)  endNodeId--;
            }
        } else {
            QVector<int> removeIds;
            for(int i = 0; i < replaceCount; i++) {
                if(replaceIds.contains(i)) continue;
                removeIds << i;
            }
            for(int relId : removeIds) {
                const int idValue = beginNodeId + iInc + iInc*relId;
                const int nodeId = WrappedInt(idValue, totalCount, reverse).toInt();
                edited->actionDemoteToDissolved(nodeId, false);
            }
        }
    } else if(replaceCount > currentCount) {
        QVector<int> insertIds;
        for(int i = 0; i < replaceCount; i++) {
            if(replaceIds.contains(i)) continue;
            insertIds << i;
        }
        std::sort(insertIds.begin(), insertIds.end());
        for(const int insertId : insertIds) {
            const int prevId = WrappedInt(beginNodeId + iInc*insertId,
                                          totalCount, reverse).toInt();
            const int nextId = WrappedInt(prevId + iInc,
                                          totalCount, reverse).toInt();
            const int orderedPrevId = qMin(prevId, nextId);
            const int orderedNextId = qMax(prevId, nextId);
            insertNodeBetween(orderedPrevId, orderedNextId, 0.5);
            edited->actionPromoteDissolvedNodeToNormal(nextId);
            totalCount++;
            if(beginNodeId > orderedPrevId) beginNodeId++;
            if(endNodeId > orderedPrevId) endNodeId++;
        }
    }

    int skipped = 0;
    for(int i = 0; i < replaceCount + skipped; i++) {
        const int nodeId = WrappedInt(beginNodeId + iInc + iInc*i,
                                      totalCount, reverse).toInt();
        const auto node = edited->getNodePtr(nodeId);
        if(node->isDissolved()) {
            skipped++;
            continue;
        }
        const int segId = i - skipped;
        const auto& seg = with.at(segId);
        const auto& nextSeg = with.at(segId + 1);
        const QPointF c0 = reverse ? nextSeg.c1() : seg.c2();
        const QPointF p1 = seg.p3();
        const QPointF c2 = reverse ? seg.c2() : nextSeg.c1();

        const NormalNodeData values(true, true, CtrlsMode::corner, c0, p1, c2);

        edited->actionSetNormalNodeValues(nodeId, values);
    }

    if(changeAll) {
        for(const auto &key : keys) {
            key->finishValueTransform();
        }
    }
    prp_finishTransform();

    if(changeAll) prp_afterWholeInfluenceRangeChanged();
    else changed();
}

int SmartPathAnimator::actionAddFirstNode(const QPointF &relPos) {
    return actionAddFirstNode({false, false, CtrlsMode::symmetric,
                               relPos, relPos, relPos});
}

int SmartPathAnimator::actionAddFirstNode(const NormalNodeData &data) {
    prp_pushUndoRedoName("Add New Node");

    prp_startTransform();

    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SmartPathKey*>(key);
        key->startValueTransform();
        spKey->getValue().actionAddFirstNode(data);
        key->finishValueTransform();
    }

    const int id = baseValue().actionAddFirstNode(data);
    prp_finishTransform();

    prp_afterWholeInfluenceRangeChanged();
    return id;
}
