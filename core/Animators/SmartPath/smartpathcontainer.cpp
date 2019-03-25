#include "smartpathcontainer.h"

SmartPath::SmartPath() : mNodesList() {}

SmartPath::SmartPath(const SmartPath &other) :
    SmartPath() {
    mNodesList.setNodeList(other.getNodesRef().getList());
}

void SmartPath::actionRemoveNormalNode(const int &nodeId) {
    Node * const node = mNodesList.at(nodeId);
    if(!node->isNormal())
        RuntimeThrow("Invalid node type. "
                     "Only normal nodes can be removed.");

    Node * currNode = node;
    while(currNode->getPrevNode()) {
        const int prevId = currNode->getPrevNodeId();
        currNode = mNodesList.at(prevId);
        if(currNode->isNormal()) break;
        if(currNode->isDissolved()) currNode->fT *= 0.5;
    }
    currNode = node;
    while(currNode->getNextNode()) {
        const int nextId = currNode->getNextNodeId();
        currNode = mNodesList.at(nextId);
        if(currNode->isNormal()) break;
        if(currNode->isDissolved()) currNode->fT = currNode->fT*0.5 + 0.5;
    }
}

int SmartPath::actionAddFirstNode(const QPointF &c0,
                                  const QPointF &p1,
                                  const QPointF &c2) {
    const int insertId = mNodesList.appendNode(Node(c0, p1, c2));
    return insertId;
}

int SmartPath::actionAppendNodeAtEndNode(const int &endNodeId,
                                         const NodePointValues &values) {
    Node * const endNode = mNodesList.at(endNodeId);
    if(!endNode->isNormal())
        RuntimeThrow("Invalid node type. "
                     "End nodes should always be NORMAL.");
    const Node nodeBlueprint = *endNode;
    endNode->fC0 = values.fC0;
    endNode->fP1 = values.fP1;
    endNode->fC2 = values.fC2;
    return mNodesList.insertNodeAfter(endNodeId, nodeBlueprint);
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

int SmartPath::actionInsertNodeBetween(const int &prevId,
                                      const int& nextId,
                                      const QPointF &c0,
                                      const QPointF &p1,
                                      const QPointF &c2) {
    return insertNodeBetween(prevId, nextId, Node(c0, p1, c2));
}

void SmartPath::actionPromoteDissolvedNodeToNormal(const int &nodeId) {
    mNodesList.promoteDissolvedNodeToNormal(nodeId);
}

void SmartPath::actionMoveNodeBetween(const int& movedNodeId,
                                      const int& prevNodeId,
                                      const int& nextNodeId) {
    Node * const prevNode = mNodesList.at(prevNodeId);
    if(prevNode->getNextNodeId() != nextNodeId)
        RuntimeThrow("Trying to move between not connected nodes");
    mNodesList.moveNodeAfter(movedNodeId, mNodesList.at(movedNodeId),
                             prevNodeId, prevNode);
}

void SmartPath::actionDisconnectNodes(const int &node1Id, const int &node2Id) {
    Node * const node1 = mNodesList.at(node1Id);
    Node * const node2 = mNodesList.at(node2Id);
    int prevId;
    int nextId;
    if(node1->getNextNodeId() == node2Id) {
        prevId = node1Id;
        nextId = node2Id;
    } else if(node2->getNextNodeId() == node1Id) {
        prevId = node2Id;
        nextId = node1Id;
    } else {
        RuntimeThrow("Trying to disconnect not connected nodes");
    }

    Node * const prevNode = mNodesList.at(prevId);
    if(!prevNode->isNormal()) {
        const int prevNormalIdV = mNodesList.prevNormalId(prevId);
        Node * const prevNormalNode = mNodesList.at(prevNormalIdV);
        mNodesList.setNodeType(prevNode, Node::NORMAL);
        prevNode->fC0 = prevNormalNode->fC0;
        prevNode->fP1 = prevNormalNode->fP1;
        prevNode->fC2 = prevNormalNode->fC2;
    }
    Node * const nextNode = mNodesList.at(nextId);
    if(!nextNode->isNormal()) {
        const int nextNormalIdV = mNodesList.nextNormalId(nextId);
        Node * const nextNormalNode = mNodesList.at(nextNormalIdV);
        mNodesList.setNodeType(nextNode, Node::NORMAL);
        nextNode->fC0 = nextNormalNode->fC0;
        nextNode->fP1 = nextNormalNode->fP1;
        nextNode->fC2 = nextNormalNode->fC2;
    }
}

void SmartPath::actionConnectNodes(const int &node1Id, const int &node2Id) {
    if(node1Id == 0) {
        mNodesList.setNodeNextId(node2Id, node1Id);
        mNodesList.setNodePrevId(node1Id, node2Id);
    } else if(node2Id == 0) {
        mNodesList.setNodeNextId(node1Id, node2Id);
        mNodesList.setNodePrevId(node2Id, node1Id);
    } else RuntimeThrow("Only first and last node can be connected");
}

NodeList *SmartPath::getNodesPtr() {
    return &mNodesList;
}

SkPath SmartPath::getPathAt() const {
    return mNodesList.toSkPath();
}

ValueRange SmartPath::dissolvedTRange(const int &nodeId) {
    return {mNodesList.prevT(nodeId), mNodesList.nextT(nodeId)};
}
