#include "smartpath.h"

SmartPath::SmartPath(const SkPath &path) {
    setPath(path);
}

SmartPath::SmartPath(const NodeList &path) : mNodesList(path) {}

void SmartPath::actionOpen() {
    if(!isClosed()) return;
    mNodesList.splitNodeAndDisconnect(getNodeCount() - 1);
}

void SmartPath::actionRemoveNode(const int nodeId, const bool approx) {
    mNodesList.removeNode(nodeId, approx);
}

int SmartPath::actionAddFirstNode(const QPointF &c0,
                                  const QPointF &p1,
                                  const QPointF &c2) {
    const int insertId = mNodesList.appendNode(Node(c0, p1, c2));
    return insertId;
}

int SmartPath::actionAddFirstNode(const NormalNodeData& data) {
    const int insertId = mNodesList.appendNode(Node(data));
    return insertId;
}

int SmartPath::actionAppendNodeAtEndNode(const int endNodeId) {
    Node * const endNode = mNodesList.at(endNodeId);
    if(!endNode->isNormal())
        RuntimeThrow("Invalid node type. End nodes should always be NORMAL.");
    const NodePointValues vals = {endNode->p1(), endNode->p1(), endNode->p1()};
    return actionAppendNodeAtEndNode(vals);
}

int SmartPath::actionAppendNodeAtEndNode(const NodePointValues &values) {
    return mNodesList.appendNode(Node(values.fC0, values.fP1, values.fC2));
}

int SmartPath::actionAppendNodeAtEndNode() {
    if(mNodesList.count() == 0) RuntimeThrow("Cannot append new node ");
    return actionAppendNodeAtEndNode(mNodesList.count() - 1);
}

int SmartPath::insertNodeBetween(const int prevId,
                                 const int nextId,
                                 const Node& nodeBlueprint) {
    if(!mNodesList.nodesConnected(prevId, nextId))
        RuntimeThrow("Cannot insert between not connected nodes");
    return mNodesList.insertNodeAfter(prevId, nodeBlueprint);
}

int SmartPath::actionInsertNodeBetween(const int prevId,
                                       const int nextId,
                                       const qreal t) {
    if(prevId == nextId)
        RuntimeThrow("Cannot insert a node between a single node");

    const auto prev = getNodePtr(prevId);
    const auto next = getNodePtr(nextId);

    if(!mNodesList.nodesConnected(prevId, nextId)) {
        if(!prev->isNormal() || !next->isNormal())
            RuntimeThrow("Invalid insert between not connected nodes");
        const int prevNormalId = prev->getNodeId();
        const int nextNormalId = next->getNodeId();
        const int nDiss = prevNormalId < nextNormalId ?
                    nextNormalId - prevNormalId - 1 :
                    mNodesList.count() - 1 - prevNormalId + nextNormalId;
        const Node * iPrevNode = prev;

        for(int i = 0; i < nDiss; i++) {
            const auto iCurrNode = mNodesList.nextNode(iPrevNode);
            if(iCurrNode->t() > t) {
                return insertNodeBetween(iPrevNode->getNodeId(),
                                         iCurrNode->getNodeId(), Node(t));
            }
            iPrevNode = iCurrNode;
        }
        return insertNodeBetween(prevNodeId(nextId), nextId, Node(t));
    }

    if(prev->isDissolved() || next->isDissolved()) {
        const qreal prevT = prev->isNormal() ? 0 : prev->t();
        const qreal nextT = next->isNormal() ? 1 : next->t();

        const qreal mappedT = gMapTFromFragment(prevT, nextT, t);
        return insertNodeBetween(prevId, nextId, Node(mappedT));
    }
    return insertNodeBetween(prevId, nextId, Node(t));
}

int SmartPath::actionInsertNodeBetween(
        const int prevId, const int nextId,
        const QPointF &c0, const QPointF &p1, const QPointF &c2) {
    return insertNodeBetween(prevId, nextId, Node(c0, p1, c2));
}

void SmartPath::actionPromoteDissolvedNodeToNormal(const int nodeId) {
    mNodesList.promoteDissolvedNodeToNormal(nodeId);
}

void SmartPath::actionDemoteToDissolved(const int nodeId, const bool approx) {
    mNodesList.demoteNormalNodeToDissolved(nodeId, approx);
}

void SmartPath::actionMoveNodeBetween(const int movedNodeId,
                                      const int prevNodeId,
                                      const int nextNodeId) {
    if(!mNodesList.nodesConnected(prevNodeId, nextNodeId))
        RuntimeThrow("Trying to move between not connected nodes");
    const int targetId = (movedNodeId < prevNodeId ? prevNodeId : nextNodeId);
    mNodesList.moveNode(movedNodeId, targetId);
}

void SmartPath::actionDisconnectNodes(const int node1Id, const int node2Id) {
    int prevId;
    int nextId;
    if(nextNodeId(node1Id) == node2Id) {
        prevId = node1Id;
        nextId = node2Id;
    } else if(nextNodeId(node2Id) == node1Id) {
        prevId = node2Id;
        nextId = node1Id;
    } else {
        RuntimeThrow("Trying to disconnect not connected nodes");
    }

    Node * const prevNode = mNodesList.at(prevId);
    Node * const nextNode = mNodesList.at(nextId);

    if(prevNode->isDissolved())
        actionPromoteDissolvedNodeToNormal(prevId);
    if(nextNode->isDissolved())
        actionPromoteDissolvedNodeToNormal(nextId);

    if(isClosed()) {
        mNodesList.moveNodesToFrontStartingWith(nextId);
        mNodesList.setClosed(false);
    } else {
        mLastDetached = mNodesList.detachNodesStartingWith(nextId);
    }
}

void SmartPath::actionConnectNodes(const int node1Id, const int node2Id) {
    if((node1Id == 0 && node2Id == mNodesList.count() - 1) ||
       (node2Id == 0 && node1Id == mNodesList.count() - 1)) {
        mNodesList.setClosed(true);
    } else return;
}

void SmartPath::actionMergeNodes(const int node1Id, const int node2Id) {
    mNodesList.mergeNodes(node1Id, node2Id);
}

SkPath SmartPath::getPathAt() const {
    return mNodesList.toSkPath();
}

void SmartPath::setPath(const SkPath &path) {
    mNodesList.setPath(path);
}

qValueRange SmartPath::dissolvedTRange(const int nodeId) {
    return {mNodesList.prevT(nodeId), mNodesList.nextT(nodeId)};
}

eWriteStream &operator<<(eWriteStream &dst, const SmartPath &path) {
    path.write(dst);
    return dst;
}

eReadStream &operator>>(eReadStream &src, SmartPath &path) {
    path.read(src);
    return src;
}
