#include "smartpathcontainer.h"

SmartPath::SmartPath() : mNodesList() {}

void SmartPath::actionRemoveNode(const int &nodeId) {
    Node * const node = mNodesList.at(nodeId);
    if(node->isNormal()) {
        Node * currNode = node;
        while(mNodesList.prevNode(currNode)) {
            currNode = mNodesList.prevNode(currNode);
            if(currNode->isNormal()) break;
            if(currNode->isDissolved()) currNode->setT(currNode->t()*0.5);
        }
        currNode = node;
        while(mNodesList.nextNode(currNode)) {
            currNode = mNodesList.nextNode(currNode);
            if(currNode->isNormal()) break;
            if(currNode->isDissolved()) currNode->setT(currNode->t()*0.5 + 0.5);
        }
    }
    mNodesList.removeNodeFromList(nodeId);
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

int SmartPath::actionAppendNodeAtEndNode(const int &endNodeId) {
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

int SmartPath::insertNodeBetween(const int& prevId,
                                 const int& nextId,
                                 const Node& nodeBlueprint) {
    if(!mNodesList.nodesConnected(prevId, nextId))
        RuntimeThrow("Cannot insert between not connected nodes");
    return mNodesList.insertNodeAfter(prevId, nodeBlueprint);
}

int SmartPath::actionInsertNodeBetween(const int &prevId,
                                       const int& nextId,
                                       const qreal& t) {
    return insertNodeBetween(prevId, nextId, Node(t));
}

int SmartPath::actionInsertNodeBetween(
        const int &prevId, const int& nextId,
        const QPointF &c0, const QPointF &p1, const QPointF &c2) {
    return insertNodeBetween(prevId, nextId, Node(c0, p1, c2));
}

void SmartPath::actionPromoteDissolvedNodeToNormal(const int &nodeId) {
    mNodesList.promoteDissolvedNodeToNormal(nodeId);
}

void SmartPath::actionDemoteToDissolved(const int &nodeId, const bool& approx) {
    mNodesList.demoteNormalNodeToDissolved(nodeId, approx);
}

void SmartPath::actionMoveNodeBetween(const int& movedNodeId,
                                      const int& prevNodeId,
                                      const int& nextNodeId) {
    if(!mNodesList.nodesConnected(prevNodeId, nextNodeId))
        RuntimeThrow("Trying to move between not connected nodes");
    const int targetId = (movedNodeId < prevNodeId ? prevNodeId : nextNodeId);
    mNodesList.moveNode(movedNodeId, targetId);
}

void SmartPath::actionDisconnectNodes(const int &node1Id, const int &node2Id) {
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

void SmartPath::actionConnectNodes(const int &node1Id, const int &node2Id) {
    if((node1Id == 0 && node2Id == mNodesList.count() - 1) ||
       (node2Id == 0 && node1Id == mNodesList.count() - 1)) {
        mNodesList.setClosed(true);
    } else RuntimeThrow("Only first and last node can be connected");
}

void SmartPath::actionMergeNodes(const int &node1Id, const int &node2Id) {
    mNodesList.mergeNodes(node1Id, node2Id);
}

SkPath SmartPath::getPathAt() const {
    return mNodesList.toSkPath();
}

void SmartPath::setPath(const SkPath &path) {
    mNodesList.setPath(path);
}

ValueRange SmartPath::dissolvedTRange(const int &nodeId) {
    return {mNodesList.prevT(nodeId), mNodesList.nextT(nodeId)};
}
